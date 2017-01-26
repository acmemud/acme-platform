/**
 * Utility library for dealing with the filesystem.
 *
 * @author devo@eotl
 * @alias FileLib
 */
#pragma no_clone
#include <sys/files.h>

// TODO need to clean up old patterns periodically
// TODO externalize caching
// ([ pattern : time; result ])
private nosave mapping pattern_cache = ([ ]);

protected int file_exists(string filename);
protected int is_directory(string filename);
protected string basename(string filename);
protected string dirname(string filename);
protected string munge_filename(string filename);
protected varargs string expand_path(string pattern, mixed rel);
protected varargs mixed *expand_pattern(string pattern, object rel);
private mixed *expand_files(string *path, mixed *dirs);
private mixed *collate_files(string dir, string pattern);
protected int is_loadable(string file);
protected int is_special_dir(string path);
private int _traverse(closure callback, mixed *info, string root, 
                      mixed *args);
protected int traverse_tree(string root, closure callback, 
                            varargs mixed *args);
protected int copy_tree(string src, string dest);
protected mixed read_value(string file);
protected int write_value(string file, mixed value);

/**
 * Test whether a file exists.
 *
 * @param  filename the filename to test
 * @return          1 if the file exists, otherwise 0
 */
protected int file_exists(string filename) {
  return file_size(filename) != -1;
}

/**
 * Test whether a path is a directory.
 *
 * @param  filename the path to test
 * @return          1 if the path is a directory, otherwise 0
 */
protected int is_directory(string filename) {
  return file_size(filename) == FSIZE_DIR;
}

/**
 * Return the filename component of a complete path.
 *
 * @param  filename the path to check
 * @return          the base filename of the path
 */
protected string basename(string filename) {
  return explode(filename, "/")[<1];
}

/**
 * Return the name of a file's containing directory.
 *
 * @param  filename the path of the file to check
 * @return          the name of the containing directory
 */
protected string dirname(string filename) {
  if (!stringp(filename) || !strlen(filename)) {
    return 0;
  }
  if (filename[<1] == '/') {
    return filename[0..<2];
  }
  string *path = explode(filename, "/");
  return implode(path[0..<2], "/");
}

/**
 * Munges a filename into a standard format. 
 * 
 * @param  filename the file (or dir) name to munge
 * @return          the munged name
 */
protected string munge_filename(string filename) {
  int *result = allocate(strlen(filename));
  int slash = 0;  
  int pos = 0;
  for (int i = 0, int j = strlen(filename); i < j; i++) {
    if (filename[i] == '/') {
      if (!slash) {
        result[pos++] = '/';
        slash = 1;
      }
    } else {
      result[pos++] = filename[i];
      slash = 0;
    }
  }
  return to_string(result);
}

/**
 * Returns the absolute filename of the possibly relative filename described
 * by pattern.  Relative paths will be resolved according to rel if rel is a 
 * string, otherwise rel->query_pwd(), or dirname(object_name(rel)) if 'rel' 
 * has no shell.  'rel' defaults to this_object() if not specified.  Home
 * directories specified with '~' will also be resolved according to 'rel'.
 *
 * @param  pattern path to be expanded
 * @param  rel     optional path, or object from which relative paths should be
 *                 resolved
 * @return         the absolute path
 */
protected varargs string expand_path(string pattern, mixed rel) {
  string cwd;
  if (stringp(rel)) {
    cwd = dirname(rel);
  } else {
    if (!objectp(rel)) {
      rel = THISO;
    }
    cwd = rel->query_cwd() || dirname(object_name(rel));
  }

  if (!stringp(pattern) || !strlen(pattern)) {
    pattern = cwd;
  }

  // first make pattern absolute
  switch (pattern[0]) {
    case '/':   /* already absolute */
      break;
    case '~':   /* home dir */
      if (objectp(rel)) {
        string name = rel->query_username();
        if (pattern == "~") {
          // TODO add support for ~+ and ~-
          // TODO move homedir expansion to ShellMixin and UserTracker
          if (name) {
            pattern = UserDir + "/" + name;
          } else {
            pattern = UserDir;
          }
        } else if (pattern[1] == '/') {
          if (name) {
            pattern = sprintf("%s/%s%s", UserDir, name, pattern[1..]);
          } else {
            pattern = UserDir + pattern[1..];
          }
        } else {
          pattern = UserDir + "/" + pattern[1..];
        }
      }
      break;
    default:
      pattern = cwd + "/" + pattern;
      break;
  }

  // expand . and ..
  string *parts = explode(pattern, "/");
  string *path = ({ });
  foreach (string part : parts) {
    switch (part) {
      case ".."  : /* walk back one dir */
        path = path[0..<2];
        break;
      case "."   : /* skip */
      case ""    :
        break;
      default    :
        path += ({ part });
        break;
    }
  }

  // put it all back together
  pattern = "/" + implode(path, "/");
  return pattern;
}

/**
 * Resolve a file pattern containing possible wildcards. Wildcards may be
 * expressed as a '*' or '?' as specified by get_dir(E). Wildcard characters
 * in the middle of the path will be expanded in place (e.g.
 * "home/*&#47;workroom.c" expands to all workroom files).
 *
 * @param  pattern the file pattern to expand
 * @param  rel     optional path or object from which relative paths should be
 *                 resolved
 * @return         a list of all matching files (constrained by valid_read)
 */
protected varargs mixed *expand_pattern(string pattern, object rel) {
  pattern = expand_path(pattern, rel);
  if (pattern[<1] == '/') {
    pattern += "*";
  }
  if (member(pattern_cache, pattern)) {
    int time = time();
    if (pattern_cache[pattern, 0] == time()) {
      return pattern_cache[pattern, 1];
    }
  }

  mixed *result = expand_files(explode(pattern, "/")[1..],
                               ({ ({ 0, "", 0, 0, 0 }) }));
  pattern_cache += ([ pattern : time(); result ]);
  return result;
}

/**
 * Recursive function to resolve wildcards in the middle of a path.
 *
 * @param  path the path to resolve, split by '/'
 * @param  dirs a running array of directories to inspect for matching files
 * @return      the list of all matching files
 */
private mixed *expand_files(string *path, mixed *dirs) {
  // FUTURE add '**' support
  mixed *result = ({ });
  string pattern = "/" + path[0];

  object logger = LoggerFactory->get_logger(THISO);
  logger->trace("pattern: %O", pattern);
  logger->trace("path: %O", path);
  if (sizeof(path) > 1) {
    foreach (mixed *dir : dirs) {
      mixed *alist = collate_files(dir[1], pattern);
      if (!alist) { continue; }
      alist = order_alist(alist[1], alist[0], alist[2], alist[3], alist[4]);
      int dir_index = rmember(alist[0], FSIZE_DIR);
      alist[0] = alist[0][0..dir_index];
      alist[1] = alist[1][0..dir_index];
      alist[2] = alist[2][0..dir_index];
      alist[3] = alist[3][0..dir_index];
      alist[4] = alist[4][0..dir_index];
      result += transpose_array(alist);
    }
    logger->trace("result: %O", result);
    return expand_files(path[1..], result);
  } else {
    foreach (mixed *dir : dirs) {
      mixed *alist = collate_files(dir[1], pattern);
      if (!alist) { continue; }
      result += transpose_array(alist);
    }
    logger->trace("result: %O", result);
    return result;
  }
}

/**
 * Collate all file information for the given directory and file pattern.
 * Returns an alist for the form:
 * <pre><code>
 * ({ names, sizes, modified_dates, accessed_dates, modes })
 * </code></pre>
 *
 * @param  dir     the parent directory of the files being collated
 * @param  pattern the file pattern, may contain wildcards
 * @return         an alist of the target directory's contents
 */
private mixed *collate_files(string dir, string pattern) {
  object logger = LoggerFactory->get_logger(THISO);
  logger->trace("dir: %O", dir);
  logger->trace("pattern: %O", pattern);
  mixed *contents = get_dir(dir + pattern, GETDIR_ALL
                                          |GETDIR_PATH
                                          |GETDIR_UNSORTED);
  int size = sizeof(contents);
  if (!size) { return 0; }

  int step = 5;
  mixed *names = allocate((size / step));
  mixed *sizes = allocate((size / step));
  mixed *modified = allocate((size / step));
  mixed *accessed = allocate((size / step));
  mixed *modes = allocate((size / step));
  int pos = 0;
  for (int i = 0; i < size; i += 5) {
    // XXX workaround a driver bug that sometimes puts nulls at the end of
    // filenames
    if (!contents[i][<1]) {
      contents[i] = contents[i][0..<2];
    }
    if (contents[i][<3..<1] == "/..") { continue; }
    if (contents[i][<2..<1] == "/.") { continue; }
    names[pos] = contents[i];
    sizes[pos] = contents[i+1];
    modified[pos] = contents[i+2];
    accessed[pos] = contents[i+3];
    modes[pos] = contents[i+4];
    pos++;
  }
  names[pos..] = ({ });
  sizes[pos..] = ({ });
  modified[pos..] = ({ });
  accessed[pos..] = ({ });
  modes[pos..] = ({ });
  return ({ names, sizes, modified, accessed, modes });
}

/**
 * Return true if a file can be loaded (ends in .c basically).
 *
 * @param  file the filename
 * @return      1 if file can be loaded, otherwise 0
 */
protected int is_loadable(string file) {
  return (file[<2..<1] == ".c");
}

/**
 * Returns 1 if a path represents a "." or ".." directory.
 * 
 * @param  path          a file path
 * @return 1 if path is a special directory
 */
protected int is_special_dir(string path) {
  int len = strlen(path);
  if (!len) {
    return 0;
  }
  if (path[0] == '/') {
    if ((len >= 3) && (path[<3..<1] == "/..")) {
      return 1;
    }
    if ((len >= 2) && (path[<2..<1] == "/.")) {
      return 1;
    }
    return 0;
  } else {
    return ((path == ".") || (path == ".."));
  }
  return 0;
}

/**
 * Internal recursive function to traverse a directory tree.
 * 
 * @param  callback      a callback to run for every file and directory in a
 *                       a tree
 * @param  info          file info of the current file as returned by get_dir()
 * @param  root          the root path for resolving relative paths
 * @param  args          extra args to pass to the callback
 * @return the number of files processed
 */
private int _traverse(closure callback, mixed *info, string root, 
                      mixed *args) {
  int result = 0;
  if (apply(callback, info[0], info[0][strlen(root)..], info[1], info[2], 
            info[3], info[4], args)) {
    result = 1;
    if ((info[1] == FSIZE_DIR) && !is_special_dir(info[0])) {
      mixed *dir = get_dir(info[0] + "/", 
                           GETDIR_ALL|GETDIR_UNSORTED|GETDIR_PATH);
      for (int i = 0, int j = sizeof(dir); i < j; i += 5) {
        result += _traverse(callback, dir[i..(i + 4)], root, args);
      }
    } 
  }
  return result;
}

/**
 * Traverse a directory tree.
 * 
 * @param  root          the root file or directory to traverse
 * @param  callback      a closure to call for every file and directory under
 *                       the root with the get_dir() info and root path
 * @param  args          extra args to pass to the callback
 * @return the number of files and directories processed
 */
protected int traverse_tree(string root, closure callback, 
                            varargs mixed *args) {
  root = munge_filename(root);
  mixed *info;
  if (root[<1] == '/') {
    root = root[0..<2];
    info = get_dir(root, GETDIR_ALL|GETDIR_UNSORTED|GETDIR_PATH);
    if (sizeof(info) && (info[1] != FSIZE_DIR)) {
      return 0;
    }
  }
  else {
    info = get_dir(root, GETDIR_ALL|GETDIR_UNSORTED|GETDIR_PATH);
  }
  if (!sizeof(info)) {
    return 0;
  }
  return _traverse(callback, info, root, args);
}

/**
 * Recursively copy files and directories.
 * 
 * @param  src           the root directory to copy
 * @param  dest          the destination path
 * @return the number of files and directories copied
 */
protected int copy_tree(string src, string dest) {
  closure copy_file = (: 
    object logger = LoggerFactory->get_logger(THISO);
    string file = $1;
    string rel = $2;
    int size = $3;
    string dest = $7;

    if (size == FSIZE_DIR) {
      string to = sprintf("%s/%s", dest, rel);
      /* WTF mkdir doesn't return 1 like the manpage says
      if (!mkdir(to)) {
        logger->debug("Couldn't mkdir %O", to);
        return 0;
      }
      */
      mkdir(to);
    } else {
      string to = sprintf("%s/%s", dest, rel);
      /* WTF copy_file doesn't either 
      if (!copy_file(file, to)) {
        logger->debug("Couldn't copy file from %O to %O", file, to);
        return 0;
      }
      */
      copy_file(file, to);
    }
    return 1;
  :);

  return traverse_tree(src, copy_file, dest);
}

/**
 * Restore a value from a .val file.
 * 
 * @param  file          the .val file
 * @return the restored value
 */
protected mixed read_value(string file) {
  string data = read_file(file);
  if (!data) {
    return 0;
  }
  return restore_value(data);
}

/**
 * Save a value to a .val file.
 * 
 * @param  file          the filename to write
 * @param  value         the value to save
 * @return 1 for success, 0 for failure
 */
protected int write_value(string file, mixed value) {
  object logger = LoggerFactory->get_logger(THISO);
  /* WTF rm doesn't return 1 like the manpage says
  if (file_exists(file) && !rm(file)) {
    logger->debug("Couldn't rm file %O", file);
    return 0;
  }
  */
  rm(file);
  return write_file(file, save_value(value));
}
