
package homework7;

public class _14211288_Àî¿¡Äþ_7_NoDiscountStrategy implements _14211288_Àî¿¡Äþ_7_IPricingStrategy
{
	@Override
	public String getIndex ()
	{
		return null;
	}

	@Override
	public String getName ()
	{
		return null;
	}

	@Override
	public int getType ()
	{
		return 0;
	}

	@Override
	public String getDescription ()
	{
		return null;
	}

	@Override
	public double getSubTotal (
		int copies,
		_14211288_Àî¿¡Äþ_7_BookSpecification prodSpec)
	{
		return copies * prodSpec.getPrice ();
	}
}