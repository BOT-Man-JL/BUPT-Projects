
package homework5;

public abstract class _14211288_Àî¿¡Äþ_5_Employee
{
	private String firstName, lastName;
	private String socialSecurityNumber;

	public _14211288_Àî¿¡Äþ_5_Employee (
		String _firstName, String _lastName, String _sSN)
	{
		firstName = _firstName;
		lastName = _lastName;
		socialSecurityNumber = _sSN;
	}

	public String GetFirstName ()
	{
		return firstName;
	}
	public void SetFirstName (String val)
	{
		firstName = val;
	}

	public String GetLastName ()
	{
		return lastName;
	}
	public void SetLastName (String val)
	{
		lastName = val;
	}

	public String getSocialSecurityNumber ()
	{
		return socialSecurityNumber;
	}

	public abstract int earning ();

	@Override
	public String toString ()
	{
		return
			"Employee:" +
			"\nFirst Name: " + GetFirstName () +
			"\nLast Name: " + GetLastName () +
			"\nSocial Security Number: " + getSocialSecurityNumber ();
	}
}
