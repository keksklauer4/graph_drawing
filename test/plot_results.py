import sys
import json
import matplotlib.pyplot as plt
import numpy as np
import csv
import math

def plot_rome_graphs(seconds):
    for i in range(16):
        plot_rome_graphs_initial(i, seconds)
        plot_rome_graphs_reopt(i, seconds)
        plot_rome_graphs_time(i, seconds)

def plot_rome_graphs_initial(i, seconds):
    f_name_prefix = 'data/res/rome/' + str(seconds) + '/rome' + str(i + 1)
    f_name_suffix = '_' + str(seconds) 
    f_type_json = '000_stats.json'
    f_name = f_name_prefix + f_name_suffix + f_type_json
    #print('try to load', f_name)
    f = open(f_name)
    data = json.load(f)
    f.close()
    #print('number of initial placements = ', data['num_initial'])
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
    #print(unvalid,'placements were unvalid')
    list = []
    for key in inital_dict.keys():
        list.append((key, inital_dict[key]))
    highest_number = 0
    lowest_cr = 1000000
    highest_cr = 0
    xvals = []
    mean = 0
    for key, value in list:
        xvals.append(key)
        mean += key * value
        if(value > highest_number):
            highest_number = value
        if(key > highest_cr):
            highest_cr = key
        if(key < lowest_cr):
            lowest_cr = key
    mean /= data['num_initial']
    variance = 0
    for key, value in list:
        for i in range(value):
            variance += (key - mean) ** 2
    variance /= data['num_initial']
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
    ax.bar(*zip(*list))    
    fig.autofmt_xdate(rotation=45)
    f_name_suffix = '_initial_placements.pdf'
    f_name = f_name_prefix + f_name_suffix
    fig.savefig(f_name)
    fig.clf()
    plt.close(fig)
    f_name_suffix = '_initial_data.csv'
    f_name = f_name_prefix + f_name_suffix
    with open(f_name, 'w') as f:
        writer = csv.writer(f)
        line = ['num_initial', str(data['num_initial'])]
        writer.writerow(line)
        line = ['min_cr', str(lowest_cr)]
        writer.writerow(line)
        line = ['max_cr', str(highest_cr)]
        writer.writerow(line)
        line = ['mean', str(mean)]
        writer.writerow(line)
        line = ['variance', str(variance)]
        writer.writerow(line)
        line = ['std_deviation', str(math.sqrt(variance))]
        writer.writerow(line)
            

def plot_rome_graphs_reopt(i, seconds):
    f_name_prefix = 'data/res/rome/' + str(seconds) + '/rome' + str(i + 1)
    f_name_suffix = '_' + str(seconds) 
    f_type_json = '000_stats.json'
    f_name = f_name_prefix + f_name_suffix + f_type_json
    #print('try to load', f_name)
    f = open(f_name)
    data = json.load(f)
    f.close()
    num_reopts = data['num_reopts']
    reopt_improvement = [0,0,0,0,0]
    reopt_deterioration = [0,0,0,0,0]
    num_reopt_method = [0,0,0,0,0]
    fails = 0
    damage = 0
    for d in data['reopts']:
        if(d['cr'] <= 0):
            reopt_improvement[d['method'] - 1] += d['cr']
        else:
            reopt_deterioration[d['method'] - 1] += d['cr']
            fails += 1
            damage += d['cr']
        num_reopt_method[d['method'] - 1] += 1
    #print(reopt_improvement)
    #print(reopt_deterioration)
    #print(num_reopt_method)

    print('invocations : ', num_reopts, "\t\tfails : ", fails, "\t\tdamage : ", damage) 

def plot_rome_graphs_time(i, seconds):
    f_name_prefix = 'data/res/rome/' + str(seconds) + '/rome' + str(i + 1)
    f_name_suffix = '_' + str(seconds) 
    f_type_json = '000_stats.json'
    f_name = f_name_prefix + f_name_suffix + f_type_json
    f = open(f_name)
    data = json.load(f)
    f.close()

    names = []
    values = []
    expl = []
    name = 'time_cr'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_coll'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_local_sat'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_move_op'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_init'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_coll_re'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_gurobi'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)

    name = 'time_gurobi_build'
    if(data[name] != 0):
        values.append(data[name])
        names.append(name)
        expl.append(0.1)
    
    fig, ax = plt.subplots()

    ax.pie(values, explode =expl, labels=names, autopct='%1.1f%%')


    f_name_suffix = '_timepie.pdf'
    f_name = f_name_prefix + f_name_suffix
    fig.savefig(f_name)
    fig.clf()
    plt.close(fig)


if __name__ == '__main__':
    plot_rome_graphs(sys.argv[1])