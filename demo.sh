#!/bin/bash

# cShell Demo Script
echo "Starting cShell Demo..."

# 1. Basic Shell Features
echo -e "\n${COLOR_CYAN}1. Basic Shell Features${COLOR_RESET}"
echo "Current directory: $(pwd)"
echo "Current user: $(whoami)"
echo "System info: $(uname -a)"

# 2. Environment Variables
echo -e "\n${COLOR_CYAN}2. Environment Variables${COLOR_RESET}"
export DEMO_VAR="Hello from cShell!"
env | grep DEMO_VAR
unset DEMO_VAR

# 3. File Operations
echo -e "\n${COLOR_CYAN}3. File Operations${COLOR_RESET}"
mkdir -p demo_dir
cd demo_dir
echo "This is a test file" > test.txt
echo "Another line" >> test.txt
cat test.txt
ls -la
cd ..

# 4. Process Management
echo -e "\n${COLOR_CYAN}4. Process Management${COLOR_RESET}"
sleep 5 &
ps
jobs
fg %1

# 5. Directory Stack
echo -e "\n${COLOR_CYAN}5. Directory Stack${COLOR_RESET}"
pushd demo_dir
pwd
pushd ..
pwd
dirs
popd
popd

# 6. Text Processing
echo -e "\n${COLOR_CYAN}6. Text Processing${COLOR_RESET}"
echo "apple
banana
cherry
date" > fruits.txt
cat fruits.txt | sort
cat fruits.txt | wc -l

# 7. AI Features
echo -e "\n${COLOR_CYAN}7. AI Features${COLOR_RESET}"
ai-help ls
ai-explain "What is process management?"
ai-suggest "How to find large files?"

# 8. Network Operations
echo -e "\n${COLOR_CYAN}8. Network Operations${COLOR_RESET}"
ping -c 1 localhost
netstat -an | grep LISTEN

# 9. System Information
echo -e "\n${COLOR_CYAN}9. System Information${COLOR_RESET}"
uname -a
whoami
pwd

# 10. Cleanup
echo -e "\n${COLOR_CYAN}10. Cleanup${COLOR_RESET}"
rm -rf demo_dir
rm fruits.txt

echo -e "\n${COLOR_GREEN}Demo completed!${COLOR_RESET}" 