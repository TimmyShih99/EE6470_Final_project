###############################################################################
#
# Copyright (c) 2020 Cadence Design Systems, Inc. All rights reserved worldwide.
#
# The code contained herein is the proprietary and confidential information
# of Cadence or its licensors, and is supplied subject to a previously
# executed license and maintenance agreement between Cadence and customer.
# This code is intended for use with Cadence high-level synthesis tools and
# may not be used with other high-level synthesis tools. Permission is only
# granted to distribute the code as indicated. Cadence grants permission for
# customer to distribute a copy of this code to any partner to aid in designing
# or verifying the customer's intellectual property, as long as such
# distribution includes a restriction of no additional distributions from the
# partner, unless the partner receives permission directly from Cadence.
#
# ALL CODE FURNISHED BY CADENCE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, AND CADENCE SPECIFICALLY DISCLAIMS ANY WARRANTY OF NONINFRINGEMENT,
# FITNESS FOR A PARTICULAR PURPOSE OR MERCHANTABILITY. CADENCE SHALL NOT BE
# LIABLE FOR ANY COSTS OF PROCUREMENT OF SUBSTITUTES, LOSS OF PROFITS,
# INTERRUPTION OF BUSINESS, OR FOR ANY OTHER SPECIAL, CONSEQUENTIAL OR
# INCIDENTAL DAMAGES, HOWEVER CAUSED, WHETHER FOR BREACH OF WARRANTY,
# CONTRACT, TORT, NEGLIGENCE, STRICT LIABILITY OR OTHERWISE.
#
################################################################################

#
# Libraries
#
set LIB_PATH    "[get_install_path]/share/stratus/techlibs/GPDK045/gsclib045_svt_v4.4/gsclib045/timing"
set LIB_NAME	"slow_vdd1v2_basicCells.lib"
use_tech_lib	"$LIB_PATH/$LIB_NAME"

#
# C++ compiler options
#
set CLOCK_PERIOD "5.0"
set_attr cc_options			" -g -DCLOCK_PERIOD=$CLOCK_PERIOD"

######################################################################
# Project-level stratus_hls options
######################################################################
set_attr prints                         on              ;
set_attr message_detail			1		;# Detail level in log file: 0, 1, or 2
set_attr flatten_arrays		        all		;# Control array flattening: none is the default
set_attr unroll_loops			off		;# Control loop unrolling: default=off
set_attr balance_expr			delay		;# Control expression balancing: off, width, delay  default=off
set_attr rtl_annotation			op,stack        ;# At a minimum use "id" to allow RTL comments to be added postprocess
#set_attr dpopt_auto			op      	;# Set automatic DPOPT part creation: off, array, op, expr
set_attr inline_partial_constants	on		;# Remove constant portions of variables
set_attr lsb_trimming			on		;# Remove unused LSBs: default=off
set_attr undef_func                     error           ;# Error if C++ functions are undefined
set_attr ignore_scan_cells              on              ;# Don't use scan cells when characterizing parts
#set_attr ignore_cells			"XYZ* ABC*"	;# Define list of cells to ignore
#set_attr wireload			none		;# Control wireload model used: default=none

# Timing options
set_attr clock_period			$CLOCK_PERIOD  		    ;# Clock period is required
set_attr cycle_slack			[expr 0.10 * $CLOCK_PERIOD] ;# 10% margin in scheduling clock
set_attr path_delay_limit		111                         ;# sets allocation clock ~= clock_period

#
# System Module Configurations
#
define_system_module	main	main.cc		;# SystemC module for simulation only
define_system_module	System	system.cc	;# SystemC module for simulation only
define_system_module	tb	tb.cc			;# SystemC module for simulation only

#
# Synthesis Module Configurations
#
define_hls_module	dut	dut.cc		;# SystemC module to be synthesized
define_hls_config	dut	BASIC		;# A synthesis configuration of the hls_module
define_hls_config   dut DPA       --dpopt_auto=op,expr;

define_hls_config dut     NODPO   -DLAT=2
define_hls_config dut     DPO_AUTO_OP     -DLAT=3 --dpopt_auto=op
define_hls_config dut     DPO_AUTO_EXPR   -DLAT=3 --dpopt_auto=expr
define_hls_config dut     DPO_REGION      -DLAT=2 -DDPOPT_ALL

define_hls_config   dut FLAT_UNROLL_ALL_FAST --flatten_arrays=all -post_elab_tcl {
    unroll_loops [find -loop "*_loop" ]
    constrain_latency -max_lat $HLS::ACHIEVABLE [find -loop "while_1"]
}
define_hls_config   dut FLAT_UNROLL_ALL --flatten_arrays=all -post_elab_tcl {
    unroll_loops [find -loop "*_loop" ]
    constrain_latency [find -loop "while_1"]
}
define_hls_config   dut PIPELINE -DII=1

#
# Simulation Configurations
#
if {1} {
    # settings for builtin simulator
    use_systemc_simulator builtin
    use_verilog_simulator xcelium ;# 'xcelium' or 'vcs'
    enable_waveform_logging -vcd ;# to store signal transitions vcd, shm (for Xcelium), or fsdb
    set_systemc_options -version 2.3 -gcc 6.3 ;# enables C++14
} else {
    # settings for Xcelium simulator
    use_systemc_simulator xcelium
    enable_waveform_logging -shm ;# to store signal transitions vcd, shm (for Xcelium), or fsdb
    set_systemc_options -gcc 6.3 ;# "9.3" or "6.3"
}
set_attr end_of_sim_command "make cmp_result"	;# Make rule to run at end of each simulation

define_sim_config B			{dut BEH}			;# A behavioral PIN-level configuration

# The following rules are TCL code to create a behavioral and RTL
# simulation configuration and a power analysis config 
# for each hls_config defined.
foreach config [find -hls_config *] {
	set cname [get_attr name $config]
	
	define_sim_config ${cname}_B "dut BEH   $cname"
	define_sim_config ${cname}_V "dut RTL_V $cname"
    define_power_config P_${cname} ${cname}_V -module dut
}

######################################################################
# Genus Logic Synthesis Configuration
######################################################################
define_logic_synthesis_config L {dut -all} \
    -options \
        {BDW_LS_NOGATES 1} \
		{BDW_LS_DO_DISSOLVE 1} 

