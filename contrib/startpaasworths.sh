#!/bin/bash

VERSION=`cat /etc/worthsversion`

if [[ "$IS_BROADCAST_NODE" ]]; then
  WORTHS="/usr/local/worths-default/bin/worths"
elif [[ "$IS_AH_NODE" ]]; then
  WORTHS="/usr/local/worths-default/bin/worths"
else
  WORTHS="/usr/local/worths-full/bin/worths"
fi

chown -R worths:worths $HOME

# clean out data dir since it may be semi-persistent block storage on the ec2 with stale data
rm -rf $HOME/*

ARGS=""

# if user did pass in desired seed nodes, use
# the ones the user specified:
if [[ ! -z "$WORTHS_SEED_NODES" ]]; then
    for NODE in $WORTHS_SEED_NODES ; do
        ARGS+=" --p2p-seed-node=$NODE"
    done
fi

NOW=`date +%s`
WORTHS_FEED_START_TIME=`expr $NOW - 1209600`

ARGS+=" --follow-start-feeds=$WORTHS_FEED_START_TIME"

WORTHS_PROMOTED_START_TIME=`expr $NOW - 604800`
ARGS+=" --tags-start-promoted=$WORTHS_PROMOTED_START_TIME"

if [[ ! "$DISABLE_BLOCK_API" ]]; then
   ARGS+=" --plugin=block_api"
fi

# overwrite local config with image one
if [[ "$IS_BROADCAST_NODE" ]]; then
  cp /etc/worths/config-for-broadcaster.ini $HOME/config.ini
elif [[ "$IS_AH_NODE" ]]; then
  cp /etc/worths/config-for-ahnode.ini $HOME/config.ini
elif [[ "$IS_OPSWHITELIST_NODE" ]]; then
  cp /etc/worths/fullnode.opswhitelist.config.ini $HOME/config.ini
else
  cp /etc/worths/fullnode.config.ini $HOME/config.ini
fi

chown worths:worths $HOME/config.ini

cd $HOME

mv /etc/nginx/nginx.conf /etc/nginx/nginx.original.conf
cp /etc/nginx/worths.nginx.conf /etc/nginx/nginx.conf

# get blockchain state from an S3 bucket
echo worths: beginning download and decompress of s3://$S3_BUCKET/blockchain-$VERSION-latest.tar.lz4
finished=0
count=1
if [[ "$USE_RAMDISK" ]]; then
  mkdir -p /mnt/ramdisk
  mount -t ramfs -o size=${RAMDISK_SIZE_IN_MB:-51200}m ramfs /mnt/ramdisk
  ARGS+=" --shared-file-dir=/mnt/ramdisk/blockchain"
  # try five times to pull in shared memory file
  while [[ $count -le 5 ]] && [[ $finished == 0 ]]
  do
    rm -rf $HOME/blockchain/*
    rm -rf /mnt/ramdisk/blockchain/*
    if [[ "$IS_BROADCAST_NODE" ]]; then
      aws s3 cp s3://$S3_BUCKET/broadcast-$VERSION-latest.tar.lz4 - | lz4 -d | tar x --wildcards 'blockchain/block*' -C /mnt/ramdisk 'blockchain/shared*'
    elif [[ "$IS_AH_NODE" ]]; then
      aws s3 cp s3://$S3_BUCKET/ahnode-$VERSION-latest.tar.lz4 - | lz4 -d | tar x --wildcards 'blockchain/block*' 'blockchain/*rocksdb-storage*' -C /mnt/ramdisk 'blockchain/shared*'
    else
      aws s3 cp s3://$S3_BUCKET/blockchain-$VERSION-latest.tar.lz4 - | lz4 -d | tar x --wildcards 'blockchain/block*' -C /mnt/ramdisk 'blockchain/shared*'
    fi
    if [[ $? -ne 0 ]]; then
      sleep 1
      echo notifyalert worths: unable to pull blockchain state from S3 - attempt $count
      (( count++ ))
    else
      finished=1
    fi
  done
  chown -R worths:worths /mnt/ramdisk/blockchain
else
  while [[ $count -le 5 ]] && [[ $finished == 0 ]]
  do
    rm -rf $HOME/blockchain/*
    if [[ "$IS_BROADCAST_NODE" ]]; then
      aws s3 cp s3://$S3_BUCKET/broadcast-$VERSION-latest.tar.lz4 - | lz4 -d | tar x
    elif [[ "$IS_AH_NODE" ]]; then
      aws s3 cp s3://$S3_BUCKET/ahnode-$VERSION-latest.tar.lz4 - | lz4 -d | tar x
    else
      aws s3 cp s3://$S3_BUCKET/blockchain-$VERSION-latest.tar.lz4 - | lz4 -d | tar x
    fi
    if [[ $? -ne 0 ]]; then
      sleep 1
      echo notifyalert worths: unable to pull blockchain state from S3 - attempt $count
      (( count++ ))
    else
      finished=1
    fi
  done
fi
if [[ $finished == 0 ]]; then
  if [[ ! "$SYNC_TO_S3" ]]; then
    echo notifyalert worths: unable to pull blockchain state from S3 - exiting
    exit 1
  else
    echo notifyworthssync worthssync: shared memory file for $VERSION not found, creating a new one by replaying the blockchain
    if [[ "$USE_RAMDISK" ]]; then
      mkdir -p /mnt/ramdisk/blockchain
      chown -R worths:worths /mnt/ramdisk/blockchain
    else
      mkdir blockchain
    fi
    aws s3 cp s3://$S3_BUCKET/block_log-latest blockchain/block_log
    if [[ $? -ne 0 ]]; then
      echo notifyworthssync worthssync: unable to pull latest block_log from S3, will sync from scratch.
    else
      ARGS+=" --replay-blockchain --force-validate"
    fi
    touch /tmp/isnewsync
  fi
fi

# for appbase tags plugin loading
ARGS+=" --tags-skip-startup-update"

cd $HOME

if [[ "$SYNC_TO_S3" ]]; then
  touch /tmp/issyncnode
  chown www-data:www-data /tmp/issyncnode
fi

chown -R worths:worths $HOME/*

# let's get going
cp /etc/nginx/healthcheck.conf.template /etc/nginx/healthcheck.conf
echo server 127.0.0.1:8091\; >> /etc/nginx/healthcheck.conf
echo } >> /etc/nginx/healthcheck.conf
rm /etc/nginx/sites-enabled/default
cp /etc/nginx/healthcheck.conf /etc/nginx/sites-enabled/default
/etc/init.d/fcgiwrap restart
service nginx restart
exec chpst -uworths \
    $WORTHS \
        --webserver-ws-endpoint=127.0.0.1:8091 \
        --webserver-http-endpoint=127.0.0.1:8091 \
        --p2p-endpoint=0.0.0.0:2001 \
        --data-dir=$HOME \
        $ARGS \
        $WORTHS_EXTRA_OPTS \
        2>&1&
SAVED_PID=`pgrep -f p2p-endpoint`
echo $SAVED_PID >> /tmp/worthspid
mkdir -p /etc/service/worths
if [[ ! "$SYNC_TO_S3" ]]; then
  cp /usr/local/bin/paas-sv-run.sh /etc/service/worths/run
else
  cp /usr/local/bin/sync-sv-run.sh /etc/service/worths/run
fi
chmod +x /etc/service/worths/run
runsv /etc/service/worths
