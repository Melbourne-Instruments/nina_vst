import numpy as np
import json
import os
import time

start = time.time()
path = "/udata/nina/presets/patches/"
files = []

#find all json files in directory 
# r=root, d=directories, f = files
for r, d, f in os.walk(path):
    for file in f:
        if '.json' in file:
            files.append(os.path.join(r, file))

def getParam(json_object, name):
        return [obj for obj in json_object if obj['path']==name]

for file in files:
    print(file)
    f = open(file)
    data = json.load(f)
    f.close()
    try:
        version = data["version"]

        ##updates for version 0.2
        if(data["version"]== "0.2"):
            for section in data.keys():
                if((section == "state_a") | (section == "state_b")):
                    state = data[section]
                    try:
                        #update velocity sense scaling
                        item =getParam(state, "/daw/main/ninavst/Amp_Env_Velocity_Sense")[0]
                        print(item)
                        item["value"] = (item["value"])/2 + 0.5
                        item =getParam(state, "/daw/main/ninavst/Filt_Env_Velocity_Sense")[0]
                        print(item)
                        item["value"] = (item["value"])/2 + 0.5
                        
                        #set the default pan amount to give an LR spread with no antiphase
                        item =getParam(state, "/daw/main/ninavst/Mod_Pan_Position:Pan")[0]
                        print(item)
                        item["value"] = 0.5 + 0.125
                        
                        #update the patch version number
                        data["version"] = "0.3"
                    except:
                        print("failed 0.2 update")
            
        #updates for version 0.3 go here
        if (data["version"] =="0.3"):
            print("up to date")
            
        #file is up to date, save file
        with open(file, 'w') as output:
            json.dump(data,output, indent=4)
            print("file updated")
    except:
        print("file update failed")
end = time.time()
print(end - start)