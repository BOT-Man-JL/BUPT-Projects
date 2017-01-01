
package homework7;

import java.util.HashMap;

public class _14211288_李俊宁_7_StrategyCatalog
{
	public static String[] typeStrings = new String[] {
		"绝对值优惠策略", "百分比折扣策略", "组合策略" };

	HashMap<Integer, _14211288_李俊宁_7_IPricingStrategy> strategies;

	public _14211288_李俊宁_7_StrategyCatalog ()
	{
		strategies = new HashMap<Integer, _14211288_李俊宁_7_IPricingStrategy> ();
	}

	public HashMap<Integer, _14211288_李俊宁_7_IPricingStrategy> getStrategies ()
	{
		return strategies;
	}

	public _14211288_李俊宁_7_IPricingStrategy getStrategy (int category)
	{
		return strategies.get (category);
	}

	public void setStrategy (
		int category,
		_14211288_李俊宁_7_IPricingStrategy strategy)
	{
		strategies.put (category, strategy);
	}

	public void removeStrategy (int category)
	{
		strategies.remove (category);
	}
}