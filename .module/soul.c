/**
 * Support for items capable of communication by emoting.
 *
 * @author devo@eotl
 * @alias SoulMixin
 */
#include <capability.h>

private mapping CAPABILITIES_VAR = ([ CAP_SOUL ]);
//private string CMD_IMPORTS_VAR = PlatformBinDir "/soul.cmds";

public void setup();
int has_soul();

/**
 * Setup the SoulMixin.
 */
public void setup() {

}

/**
 * Return true to indicate this object has a soul.
 * 
 * @return 1
 */
int has_soul() {
  return 1;
}
