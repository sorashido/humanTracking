#!/usr/bin/env python

import rvo2
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# timestep, neighborDist, maxNeighbors, timeHorizon, radius, maxSpeed, velocity
sim = rvo2.PyRVOSimulator(1/60., 1.5, 5, 1.5, 2, 0.4, 2)

# position, neighborDist, maxNeighbors, timeHorizon, timeHorizonObst, adius, maxSpeed, velocity
a0 = sim.addAgent((-1, 1), 1.5, 5, 1.5, 2, 0.4, 2, (1, 0))
a1 = sim.addAgent((-1, 0), 1.5, 5, 1.5, 2, 0.4, 2, (1, 0))
a2 = sim.addAgent((-1, -1), 1.5, 5, 1.5, 2, 0.4, 2, (1, 0))

a3 = sim.addAgent((1, 1), 1.5, 5, 1.5, 2, 0.4, 2, (-1, 0))
a4 = sim.addAgent((1, 0), 1.5, 5, 1.5, 2, 0.4, 2, (-1, 0))
a5 = sim.addAgent((1, -1), 1.5, 5, 1.5, 2, 0.4, 2, (-1, 0))

o1 = sim.addObstacle([(0.1, 0.1), (-0.1, 0.1), (-0.1, -0.1)])
sim.processObstacles()

sim.setAgentPrefVelocity(a0, (1, 0))
sim.setAgentPrefVelocity(a1, (1, 0))
sim.setAgentPrefVelocity(a2, (1, 0))
sim.setAgentPrefVelocity(a3, (-1, 0))
sim.setAgentPrefVelocity(a4, (-1, 0))
sim.setAgentPrefVelocity(a5, (-1, 0))


print('Simulation has %i agents and %i obstacle vertices in it.' %
      (sim.getNumAgents(), sim.getNumObstacleVertices()))

print('Running simulation')

fig = plt.figure()
plt.xlim(-2, 2)
plt.ylim(-2, 2)

def plot(data):
    sim.doStep()
    positions = ['(%5.3f, %5.3f)' % sim.getAgentPosition(agent_no)
                 for agent_no in (a0, a1, a2, a3, a4, a5)]

    print('t=%.3f  %s' % (sim.getGlobalTime(), '  '.join(positions)))

    for agent_no in (a0, a1, a2, a3, a4, a5):
        x, y = sim.getAgentPosition(agent_no)
        plt.plot(x, y, "ro")

    for i in range(sim.getNumObstacleVertices()):
        x, y = sim.getObstacleVertex(i)
        plt.plot(x, y, "bo")

ani = animation.FuncAnimation(fig, plot, interval=100)
plt.show()

# for step in range(20):
#     sim.doStep()
#
#     positions = ['(%5.3f, %5.3f)' % sim.getAgentPosition(agent_no)
#                  for agent_no in (a0, a1, a2, a3)]
#     print('step=%2i  t=%.3f  %s' % (step, sim.getGlobalTime(), '  '.join(positions)))

# for i in range(20):
#     plot("data")
