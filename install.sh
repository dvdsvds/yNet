#!/bin/bash

echo "Installing yNet CLI..."

sudo cp cli/ynet /usr/local/bin/ynet
sudo chmod +x /usr/local/bin/ynet

echo "yNet CLI installed!"
echo ""
echo "Usage:"
echo "  ynet new <project-name>"
echo "  ynet build"
echo "  ynet run"
