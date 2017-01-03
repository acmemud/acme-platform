/**
 * The PlayerTracker keeps track of the game's players. The relationship of
 * users to players is currently one to one, but could be easily changed to
 * one to many or many to many.
 *
 * @author devo@eotl
 * @alias PlayerTracker
 */
#pragma no_clone

inherit PlayerLib;

// ([ str player_id : PlayerInfo ])
mapping players;
// ([ str user_id : ([ str player_id, ...]) ])
mapping users;
int player_counter;

void setup();
string new_player(string user_id);
mapping query_players(string user_id);
string query_last_session(string player_id);
string query_user(string player_id);
string generate_id();

/**
 * Setup the PlayerTracker.
 */
void setup() {
  players = ([ ]);
  users = ([ ]);
}

/**
 * Invoked when a new player is created.
 * 
 * @param  user_id       the user the player belongs to
 * @return the new player id
 */
string new_player(string user_id) {
  object logger = LoggerFactory->get_logger(THISO);
  string player_id = generate_id();
  players[player_id] = (<PlayerInfo> 
    id: player_id,
    user: user_id
  );
  if (!member(users, user_id)) {
    users[user_id] = ([ ]);
  }
  users[user_id] += ([ player_id ]);
  return player_id;
}

/**
 * Return a collection of all players that belong to a specified user.
 * 
 * @param  user_id       the user id owning the players
 * @return a zero-width mapping of player ids
 */
mapping query_players(string user_id) {
  return users[user_id];
}

/**
 * Get the player's last (or current) session.
 * 
 * @param  player_id     the player being queried
 * @return the session id of the player's last session, or 0 if never connected
 */
string query_last_session(string player_id) {
  if (!member(players, player_id)) {
    return 0;
  }
  return players[player_id]->last_session;
}

/**
 * Get the user that owns a specified player.
 * 
 * @param  player_id     the player being queried
 * @return the user id that owns the player
 */
string query_user(string player_id) {
  if (!member(players, player_id)) {
    return 0;
  }
  return players[player_id]->user;
}

/**
 * Generate a new player id.
 * 
 * @return a new, unique player id
 */
string generate_id() {
  return sprintf("%s#%d", program_name(THISO), ++player_counter);
}

/**
 * Constructor.
 */
void create() {
  setup();
}