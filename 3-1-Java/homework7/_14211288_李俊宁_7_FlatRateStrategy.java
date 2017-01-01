
package homework7;

public class _14211288_Àî¿¡Äþ_7_FlatRateStrategy implements _14211288_Àî¿¡Äþ_7_IPricingStrategy
{
	private String index;
	private String name;
	private double discountPerBook;

	public _14211288_Àî¿¡Äþ_7_FlatRateStrategy (
		String _index,
		String _name,
		double _discountPerBook)
	{
		index = _index;
		name = _name;
		discountPerBook = _discountPerBook;
	}

	@Override
	public String getIndex ()
	{
		return index;
	}

	@Override
	public String getName ()
	{
		return name;
	}

	@Override
	public int getType ()
	{
		return 1;
	}

	@Override
	public String getDescription()
	{
		return "" + discountPerBook;
	}

	@Override
	public double getSubTotal (
		int copies,
		_14211288_Àî¿¡Äþ_7_BookSpecification prodSpec)
	{
		double ret = copies * (prodSpec.getPrice () - discountPerBook);
		if (ret < 0) return 0;
		return ret;
	}
}