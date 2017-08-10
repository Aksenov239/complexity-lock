import os
import sys
from statistics import mean, stdev

keys = ["queue"]

src = "./src/locking_scheme"
lock = "ticket"
duration = 2000
points = [500, 1000, 5000, 10000, 50000, 100000]
parallel_factors = [0.1, 0.5, 1, 2, 4, 10, 20, 30, 50, 80]
critical_factors = [1. / 80, 1. / 50, 1. / 30, 1. / 10, 1. / 4, 1. / 2, 1, 2]
proc = [5, 10, 20, 30, 39]

def log_file(duration, proc, critical, parallel, lock):
    return "out/log/d{}/{}_{}_{}_{}.txt".format(duration, proc, critical, parallel, lock)

def run_one(duration, proc, critical, parallel, lock):
    out = log_file(duration, proc, critical, parallel, lock)
    command = "{} -time {} -proc {} -critical {} -parallel {} -lock {} > {}".format(src, duration, proc, critical, parallel, lock, out)
    print(command)
    os.system(command)

def run():
    if not os.path.isdir("out/log/d{}".format(duration)):
        os.makedirs("out/log/d{}".format(duration))

    for p in proc:
        for first in points:
            for factor in parallel_factors:
                # change parallel first
                run_one(duration, p, first, int(factor * first), lock)
 
            for factor in critical_factors:
                # change critical
                run_one(duration, p, int(factor * first), first, lock)

def parse(filename):
    inf = open(filename, 'r')
    values = dict()
    for key in keys:
        values[key] = []
    for line in inf.readlines():
        ll = line.lower()
        good = None
        for key in keys:
            if key.lower() in ll:
                good = key
        if good == None:
            continue
        value = ll.strip().split(":")[1].strip().split(" ")[0]
        values[key].append(float(value))
    return values

def data_file(duration, t, proc, point, lock):
    return "data/d{}/{}_{}_{}_{}.dat".format(duration, t, proc, point, lock)

def data():
    if not os.path.isdir("data/d{}".format(duration)):
        os.makedirs("data/d{}".format(duration))

    for p in proc:
        for first in points:
            out = open(data_file(duration, "critical", p, first, lock), 'w')
            for factor in parallel_factors:
                critical = first
                parallel = int(factor * first)
                queue = parse(log_file(duration, p, critical, parallel, lock))["queue"]
                out.write("{} {}\n".format(parallel, mean(queue)))
            out.close()

            out = open(data_file(duration, "parallel", p, first, lock), 'w')
            for factor in critical_factors:
                critical = int(factor * first)
                parallel = first
                queue = parse(log_file(duration, p, critical, parallel, lock))["queue"]
                out.write("{} {}\n".format(critical, mean(queue)))
            out.close()

if sys.argv[1] == "run":
    run()
if sys.argv[1] == "data":
    data()
