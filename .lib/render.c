/**
 * Utility library for rendering messages to the screen.
 *
 * @author devo@eotl
 * @alias RenderLib
 */
private inherit JSONLib;

protected string render_json(string topic, string message, mapping context, 
                             object sender);

/**
 * Render a message and its context to a JSON string.
 * 
 * @param  topic         the message topic
 * @param  message       the message
 * @param  context       the message context
 * @param  sender        the sender object
 * @return the rendered JSON string
 */
protected string render_json(string topic, string message, mapping context, 
                             object sender) {
  return json_encode(([
    "topic": topic,
    "message": message,
    "context": context
  ]));
}
