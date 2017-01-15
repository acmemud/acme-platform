/**
 * Avatar object for the platform zone. This will be the super-avatar to all
 * flavor avatars and is used for "user-level" accounts.
 *
 * @author devo@eotl
 * @alias PlatformAvatar
 */
virtual inherit AvatarMixin;
virtual inherit SoulMixin;

private inherit ExceptionLib;
private inherit SessionLib;
private inherit ZoneLib;

// XXX get rid of this define, use ZoneTracker->get_start_room(HomeZone)
#define WORKROOM   "workroom"

public void setup();
public mixed *try_descend(string session_id);
public void on_descend(string session_id, string subsession_id, 
                       string player_id, object room, varargs mixed *args);
protected string get_player(string user_id);
protected string get_start_room(string player_id);
protected object load_start_room(string room, string player_id);
protected string get_avatar_path(object room, string player_id);
protected string get_default_start_room(string username);

/**
 * Setup platform avatar.
 */
public void setup() {
  AvatarMixin::setup();
  SoulMixin::setup();
}

/**
 * This function is called when a new session is created for this avatar,
 * and the creating object wants to "descend" their consciousness into the
 * new session. In the platform avatar, try_descend() is responsible for 
 * preparing a new subsession to the incoming session to which will be attached
 * a flavor-level avatar for a player-level account. If no player for this
 * user exists, a new player will be created with the user's workroom as its
 * default start room. Otherwise players will start in the same room where they
 * ended their last session.
 *  
 * @param  session_id    the session to which this avatar is attached
 * @return extra args to be passed to on_descend():
 *         subsession_id   the subsession for the player-level avatar
 *         player_id       the player's player id
 *         room            the player's start room
 * @throws Exception       if something goes wrong to prevent descension
 */
public mixed *try_descend(string session_id) {
  object logger = LoggerFactory->get_logger(THISO);
  mixed *result = AvatarMixin::try_descend(session_id);
  string user_id = SessionTracker->query_user(session_id);
  string username = UserTracker->query_username(user_id);
  string err;
  
  string player_id = get_player(user_id);
  if (!player_id) {
    throw((<Exception> 
      message: sprintf("no player found for user %O %O", user_id, username)
    ));
  }
  
  object avatar, room;
  string subsession_id = PlayerTracker->get_last_session(player_id);

  if (subsession_id 
      && is_active(subsession_id)
      && is_subsession(session_id, subsession_id)) {
    // use avatar and room from session already in progress
    avatar = SessionTracker->query_avatar(subsession_id);
    room = ENV(avatar);
  } else {
    // load start room
    string start_room = get_start_room(player_id);
    err = catch(room = load_start_room(start_room, player_id); publish);
    if (!room) {
      throw((<Exception> 
        message: sprintf("unable to load start room %O %O %O", 
                         player_id, start_room, err)
      ));
    }

    // clone new avatar
    string avatar_path = get_avatar_path(room, player_id);
    err = catch(avatar = clone_object(avatar_path); publish);
    if (!avatar) {
      throw((<Exception> 
        message: sprintf("unable to clone avatar %O %O %O", 
                         player_id, avatar_path, err)
      ));
    }

    // start new session
    subsession_id = SessionTracker->new_session(user_id, session_id);
    if (!subsession_id) {
      throw((<Exception> 
        message: sprintf("failed to start player session %O %O", 
                         user_id, session_id)
      ));
    }
    SessionTracker->set_avatar(subsession_id, avatar);
  }

  // avatar->try_descend
  mixed *args, ex;
  if (ex = catch(args = avatar->try_descend(subsession_id); publish)) {
    logger->warn("caught exception in try_descend: %O", ex);
    result = ({ 0, 0, 0 }) + result;
  } else {
    result = ({ subsession_id, player_id, room }) + result;
  }

  return result;
}

/**
 * Called upon a successful "descension". Descension and ascension are the 
 * processes by which multiple avatars are linked together across sessions. 
 * Descension establishes these links, and ascension removes them. A connected 
 * avatar need not necessarily itself be interactive, but may be linked to at 
 * least one other session whose avatar is interactive. The primary job of the 
 * platform avatar's on_descend() function is to perform a second descension 
 * from the user-level session to the player-level session. This involves
 * resuming the player-level session and invoking its avatar's on_descend()
 * function.
 * 
 * @param  session_id    the session this avatar has been bound to, it will be
 *                       running if it wasn't already prior to descension
 * @param  subsession_id the player-level session, may not yet be running
 * @param  player_id     the player id of the player to descend into
 * @param  room          the start room of the player avatar
 * @param  args          extra args to pass onto the player's on_descend()
 */
public void on_descend(string session_id, string subsession_id, 
                       string player_id, object room, varargs mixed *args) {
  object logger = LoggerFactory->get_logger(THISO);
  AvatarMixin::on_descend(session_id);

  object avatar = SessionTracker->query_avatar(subsession_id);  
  if (avatar) {
    if (!SessionTracker->resume_session(subsession_id)) {
      logger->warn("failed to resume player session: %O %O", 
                   player_id, subsession_id);
      return;
    }
    apply(#'call_other, avatar, "on_descend", 
          subsession_id, player_id, room, args);
  }

  return;
}

/**
 * Get a player to descend into. There is currently a constraint of one
 * player per user, so it will automatically be used for the specified user,
 * or created if one doesn't exist.
 *  
 * @param  user_id       the user id for whom to get a player
 * @return the player id of the user's player
 */
protected string get_player(string user_id) {
  mapping player_ids = PlayerTracker->query_players(user_id);
  if (!player_ids || !sizeof(player_ids)) {
    return PlayerTracker->new_player(user_id);
  } else {
    return m_indices(player_ids)[0];
  }
}

/**
 * Get the path to the player's start room. This will be the last room of the
 * player's last session, or a default.
 * 
 * @param  player_id     the player who needs a start room
 * @return the path of the player's start room
 */
protected string get_start_room(string player_id) {
  string result;
  string last_session = PlayerTracker->query_last_session(player_id);
  if (last_session) {
    result = SessionTracker->query_last_room(last_session);
  } else {
    string username = UserTracker->query_username(
                        PlayerTracker->query_user(player_id));
    result = get_default_start_room(username);
  }
  return result;
}

/**
 * Load the specified room and return it, or fall back on a default start room
 * for the specified player if the room can't be loaded. Make sure the system
 * default start room always loads and its flavor provides a valid avatar!
 * 
 * @param  room          the start room the load
 * @param  player_id     a player to use for determining reasonable default
 * @return the loaded start room
 */
protected object load_start_room(string room, string player_id) {
  object result = load_object(room);
  if (!result) {
    string username = UserTracker->query_username(
                        PlayerTracker->query_user(player_id));
    string default_room = get_default_start_room(username);
    if (default_room != room) {
      result = load_object(default_room);
    }
  }
  return result;
}

/**
 * Get the path to avatar object to use for descending the specfied player 
 * into the specified start room. This is based on the flavor of the start 
 * room, it is up to every flavor to provide a valid avatar.
 * 
 * @param  room          the player's start room
 * @param  player_id     the player that needs an avatar
 * @return the path to a valid avatar program
 */
protected string get_avatar_path(object room, string player_id) {
  string zone = get_zone(room);
  string flavor = ZoneTracker->query_flavor(zone);
  if (!flavor) {
    throw((<Exception> 
      message: sprintf("unable to deterimine avatar flavor for zone %O", zone)
    ));    
  }
  string avatar_path = FlavorTracker->query_avatar(flavor, player_id);
  if (!avatar_path) {
    throw((<Exception> 
      message: sprintf("unable to deterimine avatar path for flavor %O", 
                       flavor)
    ));    
  }
  return avatar_path;
}

/**
 * Get the user's default start room. Right now this is always the user's
 * workroom. 
 * 
 * @param  username      the username for the start room path
 * @return the default start room path
 */
protected string get_default_start_room(string username) {
  return sprintf("%s/%s/%s.c", UserDir, username, WORKROOM);
}

/**
 * Constructor. Run setup().
 */
public void create() { 
  setup();
}
