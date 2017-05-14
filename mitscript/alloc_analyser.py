import sys

from collections import defaultdict

def parse_trace(filename):
    generations = defaultdict(lambda: {'allocs': set([]), 'deallocs': set([])})
    with open(filename) as f:
        current_generation = 0
        for line in f:
            action, memory = line.strip().split(',')
            if action == 'A':
                if len(generations[current_generation]['deallocs']) > 0:
                    current_generation += 1
                generations[current_generation]['allocs'].add(memory)
            elif action == 'D':
                generations[current_generation]['deallocs'].add(memory)
    return generations


if __name__ == "__main__":
    generations = parse_trace(sys.argv[1])
    for g in range(len(generations)):
        generation = generations[g]
        print "===== Generation {}".format(g)
        print "Total allocs: {}".format(len(generation['allocs']))
        print "Immediately dealloced: {}".format(len(generation['allocs'] & generation['deallocs']))
        print "'Leaked' deallocs: {}".format(len(generation['deallocs']) - len(generation['allocs'] & generation['deallocs']) )
        print "Total deallocs: {}".format(len(generation['deallocs']))

