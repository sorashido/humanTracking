#!/usr/bin/env python

import rvo2
import matplotlib.pyplot as plt
import matplotlib.animation as animation

sim = rvo2.PyRVOSimulator(1/60., 1.5, 5, 1.5, 2, 0.4, 2)

# Pass either just the position (the other parameters then use
# the default values passed to the PyRVOSimulator constructor),
# or pass all available parameters.
a0 = sim.addAgent((0, 0))
a1 = sim.addAgent((1, 0))
a2 = sim.addAgent((1, 1))
a3 = sim.addAgent((0, 1), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))

# Obstacles are also supported.
o1 = sim.addObstacle([(0.1, 0.1), (-0.1, 0.1), (-0.1, -0.1)])
sim.processObstacles()

sim.setAgentPrefVelocity(a0, (1, 1))
sim.setAgentPrefVelocity(a1, (-1, 1))
sim.setAgentPrefVelocity(a2, (-1, -1))
sim.setAgentPrefVelocity(a3, (1, -1))

print('Simulation has %i agents and %i obstacle vertices in it.' %
      (sim.getNumAgents(), sim.getNumObstacleVertices()))

print('Running simulation')

fig = plt.figure()
plt.xlim(0, 1.0)
plt.ylim(0, 1.0)

def plot(data):
    sim.doStep()
    positions = ['(%5.3f, %5.3f)' % sim.getAgentPosition(agent_no)
                 for agent_no in (a0, a1, a2, a3)]

    print('t=%.3f  %s' % (sim.getGlobalTime(), '  '.join(positions)))

    for agent_no in (a0, a1, a2, a3):
        x, y = sim.getAgentPosition(agent_no)
        plt.plot(x, y, "ro")
    # ims.append(im)

ani = animation.FuncAnimation(fig, plot, interval=100)
plt.show()

# for i in range(20):
#     plot("data")
