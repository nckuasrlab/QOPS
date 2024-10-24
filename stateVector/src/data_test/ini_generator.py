import configparser
import os

def create_ini(setting,path):
    f=open(path,'w')
    print('[system]',file=f)
    for key in setting.keys():
        print(f'{key}={setting[key]}',file=f)
    f.close()

def set_ini(setting,key,new_value):
    setting[key]=str(new_value)


# set_ini(setting,'total_qbit',25)
# create_ini(setting,'test.ini')
