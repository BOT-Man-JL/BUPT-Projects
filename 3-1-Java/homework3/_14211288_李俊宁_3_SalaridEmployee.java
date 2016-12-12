
package homework3;

public class _14211288_Àî¿¡Äþ_3_SalaridEmployee extends _14211288_Àî¿¡Äþ_3_Employee
{
	private int weeklySalary;

	public _14211288_Àî¿¡Äþ_3_SalaridEmployee (
		String _firstName, String _lastName, String _sSN, int _weeklySalary)
	{
		super(_firstName, _lastName, _sSN);
		weeklySalary = _weeklySalary;
	}
	
	public int GetWeeklySalary ()
	{
		return weeklySalary;
	}
	public void SetWeeklySalary (int _weeklySalary)
	{
		weeklySalary = _weeklySalary;
	}
	
	@Override
	public int earning ()
	{
		return weeklySalary * 4;
	}

	@Override
	public String toString ()
	{
		return "Salarid Employee:" +
			"\nFirst Name: " + GetFirstName () +
			"\nLast Name: " + GetLastName () +
			"\nSocial Security Number: " + GetSocialSecurityNumber () +
			"\nSalary: " + GetWeeklySalary () +
			"\nEarning: " + earning ();
	}
}
