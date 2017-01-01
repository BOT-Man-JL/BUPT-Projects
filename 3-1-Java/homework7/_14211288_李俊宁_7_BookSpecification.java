
package homework7;

public class _14211288_Àî¿¡Äþ_7_BookSpecification
{
	private String isbn;
	private String title;
	private double price;
	private int category;

	public _14211288_Àî¿¡Äþ_7_BookSpecification (
		String _isbn,
		String _title,
		double _price,
		int _category)
	{
		isbn = _isbn;
		title = _title;
		price = _price;
		category = _category;
	}

	public String getISBN ()
	{
		return isbn;
	}

	public String getTitle ()
	{
		return title;
	}

	public double getPrice ()
	{
		return price;
	}

	public int getCategory ()
	{
		return category;
	}
}