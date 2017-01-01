
package homework7;

import java.util.ArrayList;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class _14211288_李俊宁_7_BuyUI extends JFrame
{
	private _14211288_李俊宁_7_Controller controller;
	private _14211288_李俊宁_7_Sale sale;
	private _14211288_李俊宁_7_ShoppingCartUI shoppingCartUI;

	public _14211288_李俊宁_7_BuyUI (_14211288_李俊宁_7_Controller _controller)
	{
		controller = _controller;
		sale = controller.getSale ();

		initUI ();

		shoppingCartUI = new _14211288_李俊宁_7_ShoppingCartUI (sale);
		sale.registerObserver (shoppingCartUI);
	}

	private void initUI ()
	{
		ArrayList<_14211288_李俊宁_7_BookSpecification> books =
			controller.getBooks ();
		String[] categoryStrings = controller.getCategoryStrings ();

		JPanel panel = new JPanel ();
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

		// Input
		fpanel.add (new JLabel ("ISBN"));
		JTextField tISBN = new JTextField (10);
		fpanel.add (tISBN);

		fpanel.add (new JLabel ("数量"));
		JTextField tCopies = new JTextField (5);
		fpanel.add (tCopies);

		// Buttons
		JButton bBuy = new JButton("购买");
		bBuy.addActionListener ((e) ->
		{
			try
			{
				controller.buyBook (
					tISBN.getText (),
					Integer.parseInt (tCopies.getText ())
				);
				shoppingCartUI.setVisible (true);
			}
			catch (NumberFormatException ex)
			{
				System.out.println ("数量 非法输入");
			}
			catch (Exception ex)
			{
				System.out.println (ex.getMessage ());
			}
		});
		JButton bShowCart = new JButton("购物车");
		bShowCart.addActionListener ((e) ->
		{
			shoppingCartUI.setVisible (true);
		});
		JButton bCancel = new JButton("离开");
		bCancel.addActionListener ((e) ->
		{
			disposeShoppingCart ();
			this.dispose();
		});
		fpanel.add (bBuy);
		fpanel.add (bShowCart);
		fpanel.add (bCancel);

		panel.add (fpanel);
		
		// Add Panel
		this.add (panel);
		this.pack ();

		// Misc
		setTitle ("购书模块");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (DISPOSE_ON_CLOSE);

		// Handle Close Window
		addWindowListener (new WindowAdapter ()
		{
            @Override
            public void windowClosing (WindowEvent e)
			{
				disposeShoppingCart ();
				super.windowClosing (e);
            }
        });
	}

	private void disposeShoppingCart ()
	{
		sale.removeObserver (shoppingCartUI);
		shoppingCartUI.dispose ();
	}
}