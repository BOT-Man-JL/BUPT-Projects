
package homework5;

public class _14211288_Àî¿¡Äþ_5_HourlyEmployee extends _14211288_Àî¿¡Äþ_5_Employee
{
	private int wage;
	private int hours;

	public _14211288_Àî¿¡Äþ_5_HourlyEmployee (
		String _firstName, String _lastName, String _sSN, int _wage, int _hours)
	{
		super(_firstName, _lastName, _sSN);
		wage = _wage;
		hours = _hours;
	}
	
	public int GetWage ()
	{
		return wage;
	}
	public void SetWage (int _wage)
	{
		wage = _wage;
	}

	public int GetHours ()
	{
		return hours;
	}
	public void SetHours (int _hours)
	{
		hours = _hours;
	}
	
	@Override
	public int earning ()
	{
		return wage * hours;
	}

	@Override
	public String toString ()
	{
		return "Hourly Employee:" +
			"\nFirst Name: " + GetFirstName () +
			"\nLast Name: " + GetLastName () +
			"\nSocial Security Number: " + getSocialSecurityNumber () +
			"\nWage: " + GetWage () +
			"\nHours: " + GetHours () +
			"\nEarning: " + earning ();
	}
}
