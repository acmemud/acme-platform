/**
 * A module to support chains of avatars.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */
#include <capability.h>
#include <command_giver.h>

inherit CommandGiverMixin;
inherit SensorMixin;
inherit ShellMixin;

inherit ArrayLib;
inherit ExceptionLib;

private mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
private string CMD_IMPORTS_VAR = PlatformBinDir "/avatar/avatar.cmds";

string master_session;
mapping slave_sessions;

protected void setup();
public mixed *try_descend(string session_id);
public void on_descend(string session_id);
protected int set_master_session(string session_id);
public string query_master_session();
protected int add_slave_session(string session_id);
protected int remove_slave_session(string session_id);
public mapping query_slave_sessions();
public string query_username();
public nomask int is_avatar();

/**
 * Setup the AvatarMixin.
 */
protected void setup() {
  CommandGiverMixin::setup();
  SensorMixin::setup();
  ShellMixin::setup();
  master_session = 0;
  slave_sessions = ([ ]);
}

/**
 * This function is called when a new session is created for this avatar,
 * and the creating object wants to "descend" their consciousness into the
 * new session. It can prevent the descension from happening, or otherwise
 * prepare for the descension that's about to take place. The try_descend() 
 * implementation at the core level does nothing but return an empty array to
 * ensure compabitility.
 * 
 * @param  session_id    the session to which this avatar is attached
 * @return extra args to be passed to on_descend():
 * @throws Exception     if something goes wrong to prevent descention
 */
public mixed *try_descend(string session_id) {
  return ({ });
}

/**
 * Called upon a successful "descension". Descension and ascension are the 
 * processes by which multiple avatars are linked together across sessions. 
 * Descension establishes these links, and ascension removes them. A connected 
 * avatar need not necessarily itself be interactive, but may be linked to at 
 * least one other session whose avatar is interactive. The primary job of 
 * AvatarMixin's core on_descend() function store the session for reference and
 * configure the other mixins. The master session will be updated if the new
 * session belongs to the same user, otherwise a slave session will be added.
 * 
 * @param  session_id    the session this avatar has been bound to, it will be
 *                       running if it wasn't already prior to descension
 */
public void on_descend(string session_id) {
  if (master_session) {
    string master_user = SessionTracker->query_user(master_session);
    string user = SessionTracker->query_user(session_id);
    string username = UserTracker->query_username(user);
    if (UserTracker->query_username(master_user) == username) {
      add_slave_session(master_session);
      set_master_session(session_id);
    } else {
      add_slave_session(session_id);
    }
  } else {
    set_master_session(session_id);
  }
  set_homedir(UserDir "/" + query_username());
  set_cwd(query_homedir());
}

/**
 * Set the master session. If the new master session was previously a slave, it
 * will no longer be a slave.
 * 
 * @param session_id the new master session id
 * @return 1 for success, 0 for failure
 */
protected int set_master_session(string session_id) {
  master_session = session_id;
  remove_slave_session(session_id);
  return 1;
}

/**
 * Get the master session.
 * 
 * @return the master session id
 */
public string query_master_session() {
  return master_session;
}

/**
 * Add a new slave session.
 * 
 * @param session_id the slave session id to add
 * @return 1 for success, 0 for failure
 */
protected int add_slave_session(string session_id) {
  slave_sessions += ([ session_id ]); 
  return 1;
}

/**
 * Remove a slave session.
 * 
 * @param  `session_id    the slave session id to remove
 * @return `[description]
 */
protected int remove_slave_session(string session_id) {
  slave_sessions -= ([ session_id ]);
  return 1;
}

/**
 * Get the slave sessions.
 * 
 * @return a zero-width mapping of slave session ids
 */
public mapping query_slave_sessions() {
  return slave_sessions;
}

/**
 * Return the username associated with this avatar. This will be the username
 * attached to the master session.
 *
 * @return the username
 */
public string query_username() {
  if (!master_session) {
    return 0;
  }
  string user_id = SessionTracker->query_user(master_session);
  return UserTracker->query_username(user_id);
}

/**
 * AvatarMixin implementation of query_terminal_type() to get terminal info
 * from connection details. TODO code this
 * 
 * @return the terminal type
 */
string query_terminal_type() {
  return SensorMixin::query_terminal_type();
}

/**
 * Returns true to designate that this object represents an avatar.
 *
 * @return 1
 */
public nomask int is_avatar() {
  return 1;
}
