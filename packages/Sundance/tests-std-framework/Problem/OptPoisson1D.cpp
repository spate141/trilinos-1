#include "Sundance.hpp"

/** 
 * Solves the Poisson equation in 1D
 */

bool leftPointTest(const Point& x) {return fabs(x[0]) < 1.0e-10;}
bool rightPointTest(const Point& x) {return fabs(x[0]-4.0*atan(1.0)) < 1.0e-10;}

int main(int argc, void** argv)
{
  
  try
		{
      MPISession::init(&argc, &argv);
      int np = MPIComm::world().getNProc();

      /* We will do our linear algebra using Epetra */
      VectorType<double> vecType = new EpetraVectorType();

      /* Create a mesh. It will be of type BasisSimplicialMesh, and will
       * be built using a PartitionedLineMesher. */
      MeshType meshType = new BasicSimplicialMeshType();
			const double pi = 4.0*atan(1.0);
      MeshSource mesher = new PartitionedLineMesher(0.0, pi, 40*np, meshType);
      Mesh mesh = mesher.getMesh();

      /* Create a cell filter that will identify the maximal cells
       * in the interior of the domain */
      CellFilter interior = new MaximalCellFilter();
      CellFilter points = new DimensionalCellFilter(0);
      CellPredicate leftPointFunc = new PositionalCellPredicate(leftPointTest);
      CellFilter left = points.subset(leftPointFunc);
      CellPredicate rightPointFunc 
        = new PositionalCellPredicate(rightPointTest);
      CellFilter right = points.subset(rightPointFunc);

      
      /* Create unknown and test functions, discretized using first-order
       * Lagrange interpolants */
      Expr u = new UnknownFunction(new Lagrange(2), "u");
      Expr v = new TestFunction(new Lagrange(2), "v");
      Expr lambda = new UnknownFunction(new Lagrange(2), "lambda");
      Expr mu = new TestFunction(new Lagrange(2), "mu");
      Expr alpha = new UnknownFunction(new Lagrange(2), "alpha");
      Expr beta = new TestFunction(new Lagrange(2), "beta");

      /* Create differential operator and coordinate function */
      Expr dx = new Derivative(0);
      Expr x = new CoordExpr(0);

      /* We need a quadrature rule for doing the integrations */
      QuadratureFamily quad = new GaussianQuadrature(4);

      /* the target we're trying to match */
      Expr target = sin(x);
      /* regularization weight */
      double R = 0.1;

      /* Define the weak form */
      Expr stateEqn = Integral(interior, (dx*mu)*(dx*u) + alpha*mu, quad);
      Expr adjointEqn = Integral(interior, (dx*v)*(dx*lambda) + v*(u-target), 
                                 quad);
      Expr inversionEqn = Integral(interior, beta*(R*alpha + lambda), quad);
      
      Expr eqn = stateEqn + adjointEqn + inversionEqn;

      /* Define the Dirichlet BC */
      Expr bc = EssentialBC(left+right, mu*u + lambda*v, quad);

      /* We can now set up the linear problem! */
      LinearProblem prob(mesh, eqn, bc, List(mu,v,beta),
                         List(u,lambda,alpha), vecType);

       /* Create an Aztec solver for solving the linear subproblems */
      std::map<int,int> azOptions;
      std::map<int,double> azParams;
      
      azOptions[AZ_solver] = AZ_gmres;
      azOptions[AZ_precond] = AZ_dom_decomp;
      azOptions[AZ_subdomain_solve] = AZ_ilu;
      azOptions[AZ_graph_fill] = 1;
      azOptions[AZ_max_iter] = 1000;
      azParams[AZ_tol] = 1.0e-13;
      
      LinearSolver<double> solver = new AztecSolver(azOptions,azParams);

      Expr soln = prob.solve(solver);

      double u0 = 1.0/(1.0 + R);
      Expr exactU = u0*sin(x);
      Expr exactAlpha = -u0*sin(x);
      Expr exactLambda = -(u0-1.0)*sin(x);

      Expr uErr = Integral(interior,(soln[0]-exactU)*(soln[0]-exactU),quad);
      Expr alphaErr = Integral(interior,(soln[2]-exactAlpha)*(soln[2]-exactAlpha),quad);
      Expr lambdaErr = Integral(interior,(soln[1]-exactLambda)*(soln[1]-exactLambda),quad);

      FunctionalEvaluator uInt(mesh, uErr);
      FunctionalEvaluator alphaInt(mesh, alphaErr);
      FunctionalEvaluator lambdaInt(mesh, lambdaErr);

      cerr << "|u - u_exact| = " << sqrt(uInt.evaluate()) << endl;
      cerr << "|lambda - lambda_exact| = " 
           << sqrt(lambdaInt.evaluate()) << endl;
      cerr << "|alpha - alpha_exact| = " 
           << sqrt(alphaInt.evaluate()) << endl;

    }
	catch(exception& e)
		{
      cerr << e.what() << endl;
		}
  MPISession::finalize();
}
