#!/bin/bash

function tryexec() {
  cmd=$1
  for x in {{1..2}}; do
    #echo $x
    eval $cmd
    if [[ $? -eq 0 ]]; then
      return
    fi
  done
  echo "Failed to execute $cmd"
  exit 1
}
echo "first fetch latest develop code from bitbucket..."
tryexec "cd $WORKSPACE/health_monitor"
tryexec "git fetch"
tryexec "git reset --hard origin/develop"
echo "copy latest devlop code to automation scripts repo..."
tryexec "cd $WORKSPACE/automationscripts/docker/pdns_hm"
tryexec "rm -rf *"
tryexec "cp -R ../../../health_monitor/* ."
tryexec "ansible-playbook ../../ansible/hm_install.yml --tags "build""
