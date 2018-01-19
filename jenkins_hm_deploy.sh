#!/bin/bash

function tryexec() {
  cmd=$1
  for x in {{1..5}}; do
    #echo $x
    eval $cmd
    if [[ $? -eq 0 ]]; then
      return
    fi
  done
  echo "Failed to execute $cmd"
  exit 1
}
echo " checking out develop branch of automationscripts....."
tryexec "cd $WORKSPACE/automationscripts"
tryexec "git fetch"
tryexec "git reset --hard origin/develop"
echo " running health_monitor_deploy.sh script for DEVELOP branch!!!"
tryexec "./hm_deploy.sh"
