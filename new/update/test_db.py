# -*- coding: utf-8 -*- 
import requests


# fd = open("./ip")

for line in open("./ip"): 
    try: 
        r = requests.get("http://%s:5001/test"%(line.strip()))
        if r.status_code == 200:
            print(line.strip()+' DB OK')
        else:
            print(line.strip()+' DB ERROR')
    except BaseException:
        print(line.strip()+' DB ERROR')
