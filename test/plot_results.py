import sys
import json
import matplotlib.pyplot as plt
import numpy as np

def plot_rome_graphs(seconds):
    for i in range(16):
        f_name_prefix = 'data/res/rome/' + str(seconds) + '/rome' + str(i + 1)
        f_name_suffix = '_' + str(seconds) + '000_stats.json'
        f_name = f_name_prefix + f_name_suffix
        print('try to load', f_name)
        f = open(f_name)
        data = json.load(f)
        f.close()
        print('number of initial placements = ', data['num_initial'])
        inital_dict = {}
        unvalid = 0
        for d in data['initial_placements']:
            if(d['valid'] == 1):
                if(d['cr'] in inital_dict.keys()):
                    inital_dict[d['cr']] = inital_dict[d['cr']] + 1
                else:
                    inital_dict[d['cr']] = 1
            else:
                unvalid += 1
        print(unvalid,'placements were unvalid')
        list = []
        for key in inital_dict.keys():
            list.append((key, inital_dict[key]))
        highest_number = 0
        lowest_cr = 1000000
        highest_cr = 0
        xvals = []
        for key, value in list:
            xvals.append(key)
            if(value > highest_number):
                highest_number = value
            if(key > highest_cr):
                highest_cr = key
            if(key < lowest_cr):
                lowest_cr = key
        list.sort()
        xvals.sort()
        if(highest_number == 0):
            highest_number = 1
        fig, ax = plt.subplots()
        ax.axis([lowest_cr, highest_cr, 0, highest_number + 1])
        ax.get_yaxis().set_ticks(np.arange(0, highest_number + 2, 1))

        ax.get_xaxis().set_ticks(xvals)
        ax.get_yaxis().set_label('multiplicity')
        ax.get_xaxis().set_label('number of crossings')
        ax.scatter(*zip(*list))
        
        fig.autofmt_xdate(rotation=45)
        f_name_suffix = '_initial_placements.pdf'
        f_name = f_name_prefix + f_name_suffix
        fig.savefig(f_name)
        
        fig.clf()

if __name__ == '__main__':
    plot_rome_graphs(sys.argv[1])