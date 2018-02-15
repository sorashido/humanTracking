#!/usr/bin/env python

import rvo2
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# timestep, neighborDist, maxNeighbors, timeHorizon, radius, maxSpeed, velocity
sim = rvo2.PyRVOSimulator(1/5., 1.5, 5, 1.5, 2, 0.4, 10)

# position, neighborDist, maxNeighbors, timeHorizon, timeHorizonObst, radius, maxSpeed, velocity
# a0 = sim.addAgent((-1, 1), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
# a1 = sim.addAgent((-1, 0), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
# a2 = sim.addAgent((-1, -1), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
#
# a3 = sim.addAgent((1, 1), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
# a4 = sim.addAgent((1, 0), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
# a5 = sim.addAgent((1, -1), 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
#
o1 = sim.addObstacle([(0.1, 0.1), (-0.1, 0.1), (-0.1, -0.1)])
sim.processObstacles()

#
# sim.setAgentPrefVelocity(a0, (1, 0))
# sim.setAgentPrefVelocity(a1, (1, 0))
# sim.setAgentPrefVelocity(a2, (1, 0))
# sim.setAgentPrefVelocity(a3, (-1, 0))
# sim.setAgentPrefVelocity(a4, (-1, 0))
# sim.setAgentPrefVelocity(a5, (-1, 0))

agents = []
def addAgent(position, velocity):
    a = sim.addAgent(position, 1.5, 5, 1.5, 2, 0.4, 2, (0, 0))
    sim.setAgentPrefVelocity(a, velocity)
    agents.append(a)

for i in range(20):
    addAgent((-10, 5 - i * 0.5), (1, 0))

for i in range(20):
    addAgent((10, 5 - i * 0.5), (-1, 0))

cmap=plt.get_cmap("tab10")
def plot(data):
    sim.doStep()
    positions = ['(%5.3f, %5.3f)' % sim.getAgentPosition(agent_no)
                 for agent_no in agents]

    print('t=%.3f  %s' % (sim.getGlobalTime(), '  '.join(positions)))

    for agent_no in agents:
        x, y = sim.getAgentPosition(agent_no)
        c = 1 if agent_no < 20 else 0
        plt.plot(x, y, color=cmap(c), marker="*", markersize=3)

    for i in range(sim.getNumObstacleVertices()):
        x, y = sim.getObstacleVertex(i)
        plt.plot(x, y, "bo")

print('Simulation has %i agents and %i obstacle vertices in it.' %
      (sim.getNumAgents(), sim.getNumObstacleVertices()))
print('Running simulation')

fig = plt.figure()
plt.xlim(-10, 10)
plt.ylim(-10, 10)
ani = animation.FuncAnimation(fig, plot, interval=10)
plt.show()
