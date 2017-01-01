
package homework7;

import java.util.ArrayList;

public class _14211288_李俊宁_7_BookCatalog
{
	public static String[] categoryStrings = new String[] {
		"非教材类计算机图书", "教材类图书",
		"连环画类图书", "养生类图书", "其他" };

	private ArrayList<_14211288_李俊宁_7_BookSpecification> books;

	public _14211288_李俊宁_7_BookCatalog ()
	{
		books = new ArrayList<_14211288_李俊宁_7_BookSpecification> ();
	}

	public ArrayList<_14211288_李俊宁_7_BookSpecification> getBooks ()
	{
		return books;
	}

	public void addBook (_14211288_李俊宁_7_BookSpecification newBook) throws Exception
	{
		if (newBook.getCategory () > categoryStrings.length || newBook.getCategory () < 1)
			throw new Exception ("书本类型 非法");

		for (_14211288_李俊宁_7_BookSpecification book : books)
			if (book.getISBN ().equals (newBook.getISBN ()))
				throw new Exception ("ISBN 冲突");
		books.add (newBook);
	}

	public _14211288_李俊宁_7_BookSpecification getBook (String isbn) throws Exception
	{
		for (_14211288_李俊宁_7_BookSpecification book : books)
			if (book.getISBN ().equals (isbn))
				return book;
		throw new Exception ("ISBN 非法");
	}
}