#!/bin/bash

gnuplot -e "plot \"$1\"  using 1:2 w l, \"$1\" using 1:3 w l; pause -1 "
