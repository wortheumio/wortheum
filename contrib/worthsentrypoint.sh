#!/bin/bash

echo /tmp/core | tee /proc/sys/kernel/core_pattern
ulimit -c unlimited

# if we're not using PaaS mode then start worths traditionally with sv to control it
if [[ ! "$USE_PAAS" ]]; then
  mkdir -p /etc/service/worths
  cp /usr/local/bin/wortheum-sv-run.sh /etc/service/worths/run
  chmod +x /etc/service/worths/run
  runsv /etc/service/worths
elif [[ "$IS_TESTNET" ]]; then
  /usr/local/bin/pulltestnetscripts.sh
else
  /usr/local/bin/startpaasworths.sh
fi
