
package homework3;

import java.util.ArrayList;
import java.util.Random;

public class _14211288_Àî¿¡Äþ_3_Test
{
    static public void main(String[] args)
    {
		Random rand = new Random ();
		ArrayList<_14211288_Àî¿¡Äþ_3_Employee> employeeList =
			new ArrayList<_14211288_Àî¿¡Äþ_3_Employee> ();

		for (int i = 0; i < 10; ++i)
		{
			switch (rand.nextInt (4))
			{
			case 0:
				employeeList.add (
					new _14211288_Àî¿¡Äþ_3_SalaridEmployee(
						"John " + i, "Lee " + i,
						"" + rand.nextInt (1024),
						rand.nextInt (4096)
					));
				break;
			case 1:
				employeeList.add (
					new _14211288_Àî¿¡Äþ_3_HourlyEmployee(
						"John " + i, "Lee " + i,
						"" + rand.nextInt (1024),
						rand.nextInt (256),
						rand.nextInt (64)
					));
				break;
			case 2:
				employeeList.add (
					new _14211288_Àî¿¡Äþ_3_CommisionEmployee(
						"John " + i, "Lee " + i,
						"" + rand.nextInt (1024),
						rand.nextInt (256),
						rand.nextInt (128) * rand.nextDouble ()
					));
				break;
			case 3:
				employeeList.add (
					new _14211288_Àî¿¡Äþ_3_BasePlusCommisionEmployee(
						"John " + i, "Lee " + i,
						"" + rand.nextInt (1024),
						rand.nextInt (256),
						rand.nextInt (128) * rand.nextDouble (),
						rand.nextInt (1024)
					));
				break;
			default:
				System.out.println ("Unknown Runtime Error!");
				break;
			}
		}

		// Print
		for (_14211288_Àî¿¡Äþ_3_Employee employee : employeeList)
		{
			System.out.println (employee.toString ());
			System.out.println ("");
		}
	}
}
