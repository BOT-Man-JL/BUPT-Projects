
package homework7;

public class _14211288_李俊宁_7_PercentageStrategy implements _14211288_李俊宁_7_IPricingStrategy
{
	private String index;
	private String name;
	private double discountPercentage;

	public _14211288_李俊宁_7_PercentageStrategy (
		String _index,
		String _name,
		double _discountPercentage) throws Exception
	{
		if (_discountPercentage < 0 || _discountPercentage > 100)
			throw new Exception ("折扣率应为 0 ~ 100");
			
		index = _index;
		name = _name;
		discountPercentage = _discountPercentage;
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
		return 2;
	}

	@Override
	public String getDescription ()
	{
		return "" + discountPercentage;
	}

	@Override
	public double getSubTotal (
		int copies,
		_14211288_李俊宁_7_BookSpecification prodSpec)
	{
		return copies * prodSpec.getPrice () * (100 - discountPercentage) / 100.0;
	}
}