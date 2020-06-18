#!/bin/bash -e

# the logic for setting the REGISTRY_OPT is as follows:
# - if env var $REGISTRY_OPT is set, then use that
# - else if it's set via the consul-usage setting in snapctl, use that
# - finally if nothing else is set, use the DEFAULT_REGISTRY_OPT env var that 
#   is defined in the snapcraft.yaml

# example usage for a core service is to define no env vars, and we get 
# REGISTRY_OPT as "--registry "

# example usage for a device service that doesn't use consul by default (but if
# a user wants to configure it to use something will use a specific address) is
# to define DEFAULT_REGISTRY_ADDR as "consul://localhost:8500" and 

# figure out the registry address, using what's in snap config if it's there
# and defaulting to DEFAULT_REGISTRY_ADDR if it's not (which could itself be
# empty - this is the case for the core services for example)
REGISTRY_SNAPCTL_ADDR=$(snapctl get consul-address)
if [ -n "$REGISTRY_SNAPCTL_ADDR" ]; then
    REGISTRY_ADDR="$REGISTRY_SNAPCTL_ADDR"
else
    REGISTRY_ADDR="$DEFAULT_REGISTRY_ADDR"
fi

REGISTRY_SNAPCTL_OPT=$(snapctl get consul-usage)
if [ -n "$REGISTRY_SNAPCTL_OPT" ]; then
    if [ "$REGISTRY_SNAPCTL_OPT" = "true" ] ; then
        # note for core services, DEFAULT_REGISTRY_ADDR is empty because 
        # the location of consul is specified in the configuration.toml file
        # and not on the command-line, but device services don't follow that
        # and need the registry address specified as the value to the 
        # --registry argument
        REGISTRY_OPT="--registry $REGISTRY_ADDR"
    elif [ "$REGISTRY_SNAPCTL_OPT" = "false" ]; then
        # if it's false then make it the empty string
        REGISTRY_OPT=""
    fi
else
    # nothing set in snap config, so use what's defined in the 
    # snapcraft.yaml
    # first, the DEFAULT_REGISTRY_OPT if it's defined, or 
    # "--registry $DEFAULT_REGISTRY_ADDR"
    if [ -n "$DEFAULT_REGISTRY_OPT" ]; then
        REGISTRY_OPT="$DEFAULT_REGISTRY_OPT"
    else
        REGISTRY_OPT="--registry $REGISTRY_ADDR"
    fi
fi

export REGISTRY_OPT

echo "REGISTRY_OPT is $REGISTRY_OPT"

# execute the next command in the chain
exec "$@"
