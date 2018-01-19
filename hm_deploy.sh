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
echo "Call ansible playbook with deploy tag, to deploy project"
tryexec "ansible-playbook ../../ansible/hm_install.yml --tags "deploy""
