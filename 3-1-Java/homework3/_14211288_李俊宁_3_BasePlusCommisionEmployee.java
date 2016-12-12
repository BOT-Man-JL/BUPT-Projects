
package homework3;

public class _14211288_Àî¿¡Äþ_3_BasePlusCommisionEmployee extends _14211288_Àî¿¡Äþ_3_CommisionEmployee
{
	private int baseSalary;

	public _14211288_Àî¿¡Äþ_3_BasePlusCommisionEmployee (
		String _firstName, String _lastName, String _sSN,
		int _grossSales, double _commissionRate, int _baseSalary)
	{
		super(_firstName, _lastName, _sSN, _grossSales, _commissionRate);
		baseSalary = _baseSalary;
	}
	
	public int GetBaseSalary ()
	{
		return baseSalary;
	}
	public void SetBaseSalary (int _baseSalary)
	{
		baseSalary = _baseSalary;
	}
	
	@Override
	public int earning()
	{
		return super.earning () + baseSalary;
	}

	@Override
	public String toString ()
	{
		return
			"Base Plus Commision Employee:" +
			"\nFirst Name: " + GetFirstName () +
			"\nLast Name: " + GetLastName () +
			"\nSocial Security Number: " + GetSocialSecurityNumber () +
			"\nBase Salary: " + GetBaseSalary () +
			"\nEarning: " + earning ();
	}
}
