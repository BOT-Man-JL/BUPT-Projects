
package homework5;

import java.util.HashMap;
import java.util.Random;

public class _14211288_Àî¿¡Äþ_5_Factory
{
	private HashMap<String, _14211288_Àî¿¡Äþ_5_Employee> employees;

	public _14211288_Àî¿¡Äþ_5_Factory ()
	{
		employees = new HashMap<String, _14211288_Àî¿¡Äþ_5_Employee> ();
	}

	public void initEmployees ()
	{
		Random rand = new Random ();
		for (int i = 0; i < 10; ++i)
		{
			String ssn = i + "" + rand.nextInt (9) +
							 "" + rand.nextInt (9) +
							 "" + rand.nextInt (9);

			switch (rand.nextInt (4))
			{
			case 0:
				this.addEmployee (
					new _14211288_Àî¿¡Äþ_5_SalaridEmployee(
						"John " + i, "Lee " + i, ssn,
						rand.nextInt (4096)
					));
				break;
			case 1:
				this.addEmployee (
					new _14211288_Àî¿¡Äþ_5_HourlyEmployee(
						"John " + i, "Lee " + i, ssn,
						rand.nextInt (256),
						rand.nextInt (64)
					));
				break;
			case 2:
				this.addEmployee (
					new _14211288_Àî¿¡Äþ_5_CommisionEmployee(
						"John " + i, "Lee " + i, ssn,
						rand.nextInt (256),
						rand.nextInt (128) * rand.nextDouble ()
					));
				break;
			case 3:
				this.addEmployee (
					new _14211288_Àî¿¡Äþ_5_BasePlusCommisionEmployee(
						"John " + i, "Lee " + i, ssn,
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
	}

	public _14211288_Àî¿¡Äþ_5_Employee getEmployee (String empSecNum)
	{
		return employees.get (empSecNum);
	}

	public _14211288_Àî¿¡Äþ_5_Employee deleteEmployee (String empSecNum)
	{
		return employees.remove (empSecNum);
	}

	public _14211288_Àî¿¡Äþ_5_Employee addEmployee (_14211288_Àî¿¡Äþ_5_Employee stu)
	{
		if (employees.get (stu.getSocialSecurityNumber ()) == null)
			return employees.put (stu.getSocialSecurityNumber (), stu);
		return null;
	}

	public _14211288_Àî¿¡Äþ_5_Employee updateEmployee (String empSecNum,
													  _14211288_Àî¿¡Äþ_5_Employee emp)
	{
		if (employees.get (empSecNum) != null)
		{
			employees.put (empSecNum, emp);
			return emp;
		}
		return null;		
	}

	public void printEmployees ()
	{
		for	(_14211288_Àî¿¡Äþ_5_Employee stu : employees.values())
		{
			System.out.println (stu.toString ());
			System.out.println ("");
		}
	}
}
