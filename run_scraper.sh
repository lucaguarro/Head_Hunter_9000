# run_scraper.sh
#!/bin/bash
source /home/luca/anaconda3/bin/activate headhunter9000
exec python /home/luca/Documents/Projects/Head_Hunter_9000/src/main.py --config "$1" --applymode "$2"
