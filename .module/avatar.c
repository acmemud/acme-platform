/**
 * A module to support chains of avatars.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */

mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
string CMD_IMPORTS_VAR = AvatarBinDir "/avatar.xml";

mixed *try_descend(string user_id) {
  return ({ });
}

void on_descend(string session_id) {
  // attach connection/superavatar
}

/**
 * Returns true to designate that this object represents an avatar.
 *
 * @return 1
 */
nomask int is_avatar() {
  return 1;
}

/**
 * Initialize AvatarMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
void setup_avatar() {
  /* change this check to top avatar
  if (interactive(THISO)) {
    ConnectionTracker->telnet_get_terminal(THISO);
    ConnectionTracker->telnet_get_NAWS(THISO);
    ConnectionTracker->telnet_get_ttyloc(THISO);
  }
  */
}
