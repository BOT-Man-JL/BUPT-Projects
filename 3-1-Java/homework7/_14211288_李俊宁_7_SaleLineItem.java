
package homework7;

public class _14211288_Àî¿¡Äþ_7_SaleLineItem
{
	private int copies;
	private _14211288_Àî¿¡Äþ_7_BookSpecification prodSpec;
	private _14211288_Àî¿¡Äþ_7_IPricingStrategy strategy;

	public _14211288_Àî¿¡Äþ_7_SaleLineItem (
		int _copies,
		_14211288_Àî¿¡Äþ_7_BookSpecification _prodSpec,
		_14211288_Àî¿¡Äþ_7_IPricingStrategy _strategy)
	{
		copies = _copies;
		prodSpec = _prodSpec;
		strategy = _strategy;
	}

	public int getCopies ()
	{
		return copies;
	}

	public String getTitle ()
	{
		return prodSpec.getTitle ();
	}

	public double getPreTotal ()
	{
		return copies * prodSpec.getPrice ();
	}

	public double getSubTotal ()
	{
		//
		// Note that: I reject to use previous interface
		//

		return strategy.getSubTotal (copies, prodSpec);
	}
}