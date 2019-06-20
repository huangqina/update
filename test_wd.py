import requests
import os
import base64
import json
import time

for line in open("./ip"): 
    try: 
        wd_url = 'http://%s:8091/el/send_string'%(line.strip())
        pic_counter = 0

        temp_dict = dict()
        temp_dict['image_id'] = 'test'
        #    temp_dict['image_id'] = string[0:-4]
        temp_dict['cell_type'] = 'mono'
        temp_dict['cell_amount'] = 13
        temp_dict['string_line'] = 'line0'
        temp_dict['create_time'] = 1554103508.3413055
        #    temp_dict['create_time'] = os.path.getctime(os.path.join(string_folder, string))
        with open(os.path.join('./', 'pic.png'), 'rb') as f:
            temp_dict['pic'] = base64.b64encode(f.read()).decode()
        response = requests.post(url=wd_url, data=json.dumps(temp_dict), headers={'content-type': 'application/json'}, timeout=20)
        if response.status_code == 200:
            print(line.strip()+' wd OK')
        else:
            print(line.strip()+' wd ERROR')
    except BaseException:
        print(line.strip()+' wd ERROR')
