
package homework5;

public class _14211288_Àî¿¡Äþ_5_CommisionEmployee extends _14211288_Àî¿¡Äþ_5_Employee
{
	private int grossSales;
	private double commissionRate;

	public _14211288_Àî¿¡Äþ_5_CommisionEmployee (
		String _firstName, String _lastName, String _sSN,
		int _grossSales, double _commissionRate)
	{
		super(_firstName, _lastName, _sSN);
		grossSales = _grossSales;
		commissionRate = _commissionRate;
	}
	
	public int GetGrossSales ()
	{
		return grossSales;
	}
	public void SetGrossSales (int _grossSales)
	{
		grossSales = _grossSales;
	}

	public double GetCommissionRate ()
	{
		return commissionRate;
	}
	public void SetCommissionRate (double _commissionRate)
	{
		commissionRate = _commissionRate;
	}
	
	@Override
	public int earning()
	{
		return (int) (grossSales * commissionRate);
	}

	@Override
	public String toString ()
	{
		return "Commision Employee:" +
			"\nFirst Name: " + GetFirstName () +
			"\nLast Name: " + GetLastName () +
			"\nSocial Security Number: " + getSocialSecurityNumber () +
			"\nGrossSales: " + GetGrossSales () +
			"\nCommissionRate: " + GetCommissionRate () +
			"\nEarning: " + earning ();
	}
}
