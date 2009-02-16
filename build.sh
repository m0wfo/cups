#!/bin/bash
make clean
ruby extconf.rb
make
sudo make install