
package homework7;

import java.util.ArrayList;
import java.awt.*;
import javax.swing.*;

public class _14211288_李俊宁_7_AddBookUI extends JFrame
{
	private _14211288_李俊宁_7_Controller controller;
	private JPanel panel;

	public _14211288_李俊宁_7_AddBookUI (_14211288_李俊宁_7_Controller _controller)
	{
		controller = _controller;
		initUI ();
	}

	private void initUI ()
	{
		update ();

		// Misc
		setTitle ("图书目录的添加模块");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (DISPOSE_ON_CLOSE);
	}

	private void update ()
	{
		ArrayList<_14211288_李俊宁_7_BookSpecification> books =
			controller.getBooks ();
		String[] categoryStrings = controller.getCategoryStrings ();
		
		// Clear previous canvas
		if (panel != null)
			this.remove (panel);

		panel = new JPanel ();
		panel.setLayout (new BoxLayout (panel, BoxLayout.Y_AXIS));

		// Scroll Pane
		JPanel spanel = new JPanel ();
		spanel.setLayout (new GridLayout (books.size () + 1, 4));
		spanel.add (new JLabel ("ISBN"));
		spanel.add (new JLabel ("书名"));
		spanel.add (new JLabel ("单价"));
		spanel.add (new JLabel ("类型"));
		for (_14211288_李俊宁_7_BookSpecification book : books)
		{
			spanel.add (new JLabel (book.getISBN ()));
			spanel.add (new JLabel (book.getTitle ()));
			spanel.add (new JLabel ("" + book.getPrice ()));
			spanel.add (new JLabel (categoryStrings[book.getCategory () - 1]));
		}
		panel.add (new JScrollPane (spanel));
		
		// Footer
		JPanel fpanel = new JPanel ();

		// ISBN
		fpanel.add (new JLabel ("ISBN"));
		JTextField tISBN = new JTextField (10);
		fpanel.add (tISBN);

		// Name
		fpanel.add (new JLabel ("书名"));
		JTextField tName = new JTextField (10);
		fpanel.add (tName);

		// Price
		fpanel.add (new JLabel ("单价"));
		JTextField tPrice = new JTextField (10);
		fpanel.add (tPrice);

		// Category
		fpanel.add (new JLabel ("类型"));
		JComboBox<String> cCategory =
			new JComboBox<String> (categoryStrings);
		fpanel.add (cCategory);

		// Buttons
		JButton bAdd = new JButton("添加");
		bAdd.addActionListener ((e) ->
		{
			try
			{
				controller.addBook (
					new _14211288_李俊宁_7_BookSpecification (
						tISBN.getText (),
						tName.getText (),
						Double.parseDouble (tPrice.getText ()),
						cCategory.getSelectedIndex () + 1
					)
				);
				update ();
			}
			catch (NumberFormatException ex)
			{
				System.out.println ("价格 非法输入");
			}
			catch (Exception ex)
			{
				System.out.println (ex.getMessage ());
			}
		});
		JButton bCancel = new JButton("取消");
		bCancel.addActionListener ((e) ->
		{
			this.dispose();
		});
		fpanel.add (bAdd);
		fpanel.add (bCancel);
		
		panel.add (fpanel);
		
		// Add Panel
		this.add (panel);
		this.pack ();
	}
}