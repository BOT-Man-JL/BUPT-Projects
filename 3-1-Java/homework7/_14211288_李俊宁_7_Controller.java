
package homework7;

import java.util.ArrayList;
import java.util.HashMap;

public class _14211288_李俊宁_7_Controller
{
	private _14211288_李俊宁_7_BookCatalog bookCatalog;
	private _14211288_李俊宁_7_StrategyCatalog strategyCatalog;
	private _14211288_李俊宁_7_Sale sale;

	public _14211288_李俊宁_7_Controller ()
	{
		bookCatalog = new _14211288_李俊宁_7_BookCatalog ();
		strategyCatalog = new _14211288_李俊宁_7_StrategyCatalog ();
		sale = new _14211288_李俊宁_7_Sale ();

		_14211288_李俊宁_7_PricingStrategyFactory
			.getInstance ().setCatalog(strategyCatalog);
	}

	// For AddBookUI

	public void addBook (_14211288_李俊宁_7_BookSpecification newBook) throws Exception
	{
		bookCatalog.addBook (newBook);
	}

	public ArrayList<_14211288_李俊宁_7_BookSpecification> getBooks ()
	{
		return bookCatalog.getBooks ();
	}

	public String[] getCategoryStrings ()
	{
		return _14211288_李俊宁_7_BookCatalog.categoryStrings;
	}

	// For StrategiesUI

	public void setStrategy (
		int category,
		_14211288_李俊宁_7_IPricingStrategy strategy)
	{
		strategyCatalog.setStrategy (category, strategy);
	}

	public void deleteStrategy (int category)
	{
		strategyCatalog.removeStrategy (category);
	}

	public HashMap<Integer, _14211288_李俊宁_7_IPricingStrategy> getStrategies ()
	{
		return strategyCatalog.getStrategies ();
	}

	public String[] getStrategyTypeStrings ()
	{
		return _14211288_李俊宁_7_StrategyCatalog.typeStrings;
	}

	// For Buy UI

	public void buyBook (String isbn, int copies) throws Exception
	{
		if (copies < 0)
			throw new Exception ("至少买一本书");

		_14211288_李俊宁_7_BookSpecification book =
			bookCatalog.getBook (isbn);

		sale.addItem (
			new _14211288_李俊宁_7_SaleLineItem (
				copies, book,
				_14211288_李俊宁_7_PricingStrategyFactory
					.getInstance ()
					.getPricingStrategy (book.getCategory ())
			)
		);
	}

	public _14211288_李俊宁_7_Sale getSale ()
	{
		return sale;
	}
}