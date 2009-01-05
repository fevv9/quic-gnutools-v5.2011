##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
if {![package vsatisfies [package provide Tcl] 8.2]} {return}
package ifneeded msgcat 1.3.3 [list source [file join $dir msgcat.tcl]]
