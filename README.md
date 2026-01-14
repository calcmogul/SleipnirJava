# Sleipnir Java bindings

This is a backport of wpimath's [Sleipnir](https://github.com/SleipnirGroup/Sleipnir/) [Java bindings](https://github.com/wpilibsuite/allwpilib/pull/8236) to 2026.

> Sparsity and Linearity-Exploiting Interior-Point solver - Now Internally Readable

Named after Odin's eight-legged horse from Norse mythology, Sleipnir is a reverse mode autodiff library and NLP solver DSL for C++23, Python, and now Java. The DSL automatically chooses the best solver based on the problem structure.

```java
import static org.wpilib.math.optimization.Constraints.eq;

import org.wpilib.math.optimization.Problem;

public class Main {
  public static void main(String[] args) {
    // Find the x, y pair with the largest product for which x + 3y = 36
    try (var problem = new Problem()) {
      var x = problem.decisionVariable();
      var y = problem.decisionVariable();

      problem.maximize(x.times(y));
      problem.subjectTo(eq(x.plus(y.times(3)), 36);
      problem.solve();

      // x = 18.0, y = 6.0
      System.out.format("x = %f, y = %f\n", x.value(), y.value());
    }
  }
}
```

## Installation

Copy this JSON into your robot project's `vendordeps` folder.

https://file.tavsys.net/sleipnir/SleipnirJava.json

## API docs

https://file.tavsys.net/sleipnir/javadoc/

## Tutorial

### Introduction

A system with position and velocity states and an acceleration input is an
example of a double integrator. We want to go from 0 m at rest to 10 m at rest
in the minimum time while obeying the velocity limit (-1, 1) and the
acceleration limit (-1, 1).

The model for our double integrator is ẍ = u where x is the vector [position;
velocity] and u is the acceleration. The velocity constraints are -1 ≤ x₁ ≤ 1
and the acceleration constraints are -1 ≤ u ≤ 1.

### Importing required libraries

```py
import static org.wpilib.math.optimization.Constraints.eq;

import static org.wpilib.math.autodiff.Variable.pow;
import static org.wpilib.math.optimization.Constraints.bounds;
import static org.wpilib.math.optimization.Constraints.eq;

import org.wpilib.math.autodiff.Variable;
import org.wpilib.math.autodiff.VariableMatrix;
import org.wpilib.math.optimization.Problem;
```

### Initializing a problem instance

First, we need to make a problem instance.

```java
final double TOTAL_TIME = 5.0; // s
final double dt = 0.005; // 5 ms
final int N = (int) (TOTAL_TIME / dt);

final double r = 2.0; // m

try (var problem = new Problem()) {
```

### Creating decision variables

First, we need to make decision variables for our state and input.

```java
// 2x1 state vector with N + 1 timesteps (includes last state)
var X = problem.decisionVariable(2, N + 1);

// 1x1 input vector with N timesteps (input at last state doesn't matter)
var U = problem.decisionVariable(1, N);
```

By convention, we use capital letters for the variables to designate
matrices.

### Applying constraints

Now, we need to apply dynamics constraints between timesteps.

```java
// Kinematics constraint assuming constant acceleration between timesteps
for (int k = 0; k < N; ++k) {
  var p_k1 = X.get(0, k + 1);
  var v_k1 = X.get(1, k + 1);
  var p_k = X.get(0, k);
  var v_k = X.get(1, k);
  var a_k = U.get(0, k);

  // pₖ₊₁ = pₖ + vₖt + 1/2aₖt²
  problem.subjectTo(eq(p_k1, p_k.plus(v_k.times(t)).plus(a_k.times(0.5 * t * t))));

  // vₖ₊₁ = vₖ + aₖt
  problem.subjectTo(eq(v_k1, v_k.plus(a_k.times(t))));
```

Next, we'll apply the state and input constraints.

```java
// Start and end at rest
problem.subjectTo(eq(X.col(0), new VariableMatrix(new double[][] {{0.0}, {0.0}})));
problem.subjectTo(eq(X.col(N), new VariableMatrix(new double[][] {{r}, {0.0}})));

// Limit velocity
problem.subjectTo(bounds(-1, X.row(1), 1));

// Limit acceleration
problem.subjectTo(bounds(-1, U, 1));
```

### Specifying a cost function

Next, we'll create a cost function for minimizing position error.

```java
// Cost function - minimize position error
var J = new Variable(0.0);
for (int k = 0; k < N + 1; ++k) {
  J = J.plus(pow(new Variable(r).minus(X.get(0, k)), 2));
}
problem.minimize(J);
```

The cost function passed to minimize() should produce a scalar output.

### Solving the problem

Now we can solve the problem.

```java
problem.solve();
```

The solver will find the decision variable values that minimize the cost
function while satisfying the constraints.

### Accessing the solution

You can obtain the solution by querying the values of the variables like so.

```py
double position = X.value(0, 0);
double velocity = X.value(1, 0);
double acceleration = U.value(0);
```

### Other applications

In retrospect, the solution here seems obvious: if you want to reach the desired
position in the minimum time, you just apply positive max input to accelerate to
the max speed, coast for a while, then apply negative max input to decelerate to
a stop at the desired position. Optimization problems can get more complex than
this though. In fact, we can use this same framework to design optimal
trajectories for a drivetrain while satisfying dynamics constraints, avoiding
obstacles, and driving through points of interest.
