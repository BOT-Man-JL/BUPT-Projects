
package homework7;

public interface _14211288_Àî¿¡Äþ_7_IPricingStrategy
{
	// Basic Properties
	public String getIndex ();
	public String getName ();
	public int getType ();
	public String getDescription ();

	//
	// Note that: I reject to use previous interface
	//

	public double getSubTotal (
		int copies,
		_14211288_Àî¿¡Äþ_7_BookSpecification prodSpec);
}