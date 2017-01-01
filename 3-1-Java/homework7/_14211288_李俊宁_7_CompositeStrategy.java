
package homework7;

import java.util.ArrayList;

public class _14211288_Àî¿¡Äþ_7_CompositeStrategy implements _14211288_Àî¿¡Äþ_7_IPricingStrategy
{
	private String index;
	private String name;
	private ArrayList<_14211288_Àî¿¡Äþ_7_IPricingStrategy> strategies;

	public _14211288_Àî¿¡Äþ_7_CompositeStrategy (
		String _index,
		String _name)
	{
		index = _index;
		name = _name;
		strategies = new ArrayList<_14211288_Àî¿¡Äþ_7_IPricingStrategy> ();
	}

	public void add (_14211288_Àî¿¡Äþ_7_IPricingStrategy strategy)
	{
		strategies.add (strategy);
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
		return 3;
	}

	@Override
	public String getDescription ()
	{
		if (strategies.size () == 0)
			return "";

		String ret = "";
		for (_14211288_Àî¿¡Äþ_7_IPricingStrategy strategy : strategies)
			ret += strategy.getIndex () + "|";
		return ret.substring (0, ret.length () - 1);
	}

	@Override
	public double getSubTotal (
		int copies,
		_14211288_Àî¿¡Äþ_7_BookSpecification prodSpec)
	{
		double ret = copies * prodSpec.getPrice ();
		for (_14211288_Àî¿¡Äþ_7_IPricingStrategy strategy : strategies)
		{
			double thisRet = strategy.getSubTotal (copies, prodSpec);
			if (thisRet < ret) ret = thisRet;
		}
		return ret;
	}
}