//Outputs Every Gear Ratio Possible - See "Gear Ratios.xlsx" for a spreadsheet of this data.
public class GearRatio 
{
	static final int MAX_AVE_ITER = 500;
	static final int MAX_MAX_VAL = 1000;
	
	//which locations the gears hit
	static boolean [][] wheelLocsHit;
	//how many times it took to hit the positions
	static int [][] timesToHit;
	//current slow gear and fast gear positions
	static int currSlow = 0, currFast = 0;
	static double min = 1000;
	//minimum value found out of all the maximum iterations values
	static double minMaxIter = 1000;

	public static void main(String [] args)
	{
		//go through every number of positions on each wheel, from 8 to 25
		for(int SLOW_SIZE = 8; SLOW_SIZE < 25; SLOW_SIZE++)
			for(int FAST_SIZE = 8; FAST_SIZE < 25; FAST_SIZE++)
			{
				//go through each gear ratio on those number of positions
				for(int ratio = 2; ratio < 25; ratio++)
				{
					//reset variables for this simulation
					wheelLocsHit = new boolean[SLOW_SIZE][FAST_SIZE];
					timesToHit = new int[SLOW_SIZE][FAST_SIZE];
					currSlow = 0;
					currFast = 0;
					int i = 0;
					
					//run until timed out or all positions hit
					while(!allHit(SLOW_SIZE, FAST_SIZE) && i < 10000)
					{
						//position isn't hit, mark it as hit
						if(!wheelLocsHit[currSlow][currFast])
						{
							wheelLocsHit[currSlow][currFast] = true;
							timesToHit[currSlow][currFast] = i;
						}
						
						//increment to next position
						i++;
						currFast += ratio;
						currSlow++;
						while(currSlow >= SLOW_SIZE) currSlow -= SLOW_SIZE;
						while(currFast >= FAST_SIZE) currFast -= FAST_SIZE;
					}
					
					//all positions were hit, output the result
					if(allHit(SLOW_SIZE, FAST_SIZE))
					{
						double aveIter = aveIter();
						int maxIter = findMaxIter();
						if(MAX_AVE_ITER >= maxIter && MAX_MAX_VAL >= maxIter)
							System.out.println(aveIter + " " + maxIter + " " + SLOW_SIZE + " " + FAST_SIZE + " " + ratio + " ");
						
					}
				}
			}
		//System.out.println("MINIMUM: " + min);
		//System.out.println("MIN MAX ITERATION: " + minMaxIter);
	}

	//returns whether all positions were hit or not
	public static boolean allHit(int slowsize, int fastsize)
	{
		for(int i = 0; i < slowsize; i++)
			for(int j = 0; j < fastsize; j++)
				if(!wheelLocsHit[i][j])
					return false;
		return true;
	}

	//prints the results on the screen
	public static String printResults()
	{
		int sum = 0;
		int maxIter = 0;
		//System.out.println("Slow\tFast\tTimes");
		for(int i = 0; i < 8; i++)
			for(int j = 0; j < 8; j++)
			{
				//System.out.println("" + i + "\t" + j + "\t" + timesToHit[i][j]);
				sum += timesToHit[i][j];
				if(timesToHit[i][j] > maxIter)
					maxIter = timesToHit[i][j];
			}

		String res = "";
		res += sum/64.0;
		res += " " + maxIter;
		if(sum/64.0 < min)
			min = sum/64.0;
		if(maxIter < minMaxIter)
			minMaxIter = maxIter;
		return res;
	}
	
	//calculates the average iterations
	public static double aveIter()
	{
		int sum = 0;
		for(int i = 0; i < 8; i++)
			for(int j = 0; j < 8; j++)
				sum += timesToHit[i][j];

		if(sum/64.0 < min)
			min = sum/64.0;
		return sum/64.0;
	}
	
	//calculates the maximum iterations out of all the locations
	public static int findMaxIter()
	{
		int maxIter = 0;
		for(int i = 0; i < 8; i++)
			for(int j = 0; j < 8; j++)
				if(timesToHit[i][j] > maxIter)
					maxIter = timesToHit[i][j];

		if(maxIter < minMaxIter)
			minMaxIter = maxIter;
		return maxIter;
	}
}
