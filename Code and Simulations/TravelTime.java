//calculates the travel iterations based on adding more characters
public class GearTurns 
{
	//min/max number of characters
	public static final int MAX_CHARS = 50;
	public static final int MIN_CHARS = 1;
	
	//maximum travel distance
	public static final int MAX_TRAVEL = 35;
	
	//how many times to run the simulations per number of characters
	public static final int NUM_TIMES = 100000;

	public static void main(String [] args)
	{
		//iterate through each character
		for(int i = MIN_CHARS; i <= MAX_CHARS; i++)
		{
			int sum = 0;
			
			//iterate through each simulation #
			for(int times = 0; times < NUM_TIMES; times++)
			{
				//finds the maximum of all the characters
				int max = 0;
				for(int j = 1; j <= i; j++)
				{
					//chance of each number 1-35 occurring is equal, so generate number
					int currMax = (int) Math.floor(Math.random() * (MAX_TRAVEL+1));
					
					//see if it's the new max
					if(currMax > max) max = currMax;
				}
				sum += max;
			}
			//output the average of the maximums
			System.out.printf("[%d, %f]\n", i, (1.0*sum/NUM_TIMES));
		}
	}
}
