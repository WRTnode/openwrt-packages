#!/bin/bash
while getopts "ab" arg
do
  case $arg in
    a)
      echo "A"
      ;;
    b)
      echo "B"
      ;;
    ?)
  exit 1
  ;;
  esac
done
