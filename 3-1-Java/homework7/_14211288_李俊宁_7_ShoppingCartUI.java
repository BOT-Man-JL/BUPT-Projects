
package homework7;

import java.util.ArrayList;
import java.awt.*;
import javax.swing.*;

public class _14211288_李俊宁_7_ShoppingCartUI extends JFrame implements _14211288_李俊宁_7_Observer
{
	private _14211288_李俊宁_7_Sale sale;
	private JScrollPane spane;

	public _14211288_李俊宁_7_ShoppingCartUI (_14211288_李俊宁_7_Sale _sale)
	{
		sale = _sale;
		initUI ();
	}

	private void initUI ()
	{
		update ();
		
		// Misc
		setTitle ("购物车");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (HIDE_ON_CLOSE);
	}

    @Override
	public void update ()
	{
		ArrayList<_14211288_李俊宁_7_SaleLineItem> items =  sale.getItems ();
		double total = sale.getTotal ();

		// Clear previous canvas
		if (spane != null)
			this.remove (spane);

		JPanel panel = new JPanel ();
		panel.setLayout (new GridLayout (items.size () + 2, 4));

		// Header
		panel.add (new JLabel ("书名"));
		panel.add (new JLabel ("数量"));
		panel.add (new JLabel ("原价"));
		panel.add (new JLabel ("现价"));

		// List
		for (_14211288_李俊宁_7_SaleLineItem item : items)
		{
			panel.add (new JLabel (item.getTitle ()));
			panel.add (new JLabel ("" + item.getCopies ()));
			panel.add (new JLabel ("" + item.getPreTotal ()));
			panel.add (new JLabel ("" + item.getSubTotal ()));
		}

		// Total Price
		panel.add (new JLabel (""));
		panel.add (new JLabel (""));
		panel.add (new JLabel ("总价"));
		panel.add (new JLabel ("" + total));
		
		// Add Panel
		spane = new JScrollPane (panel);
		this.add (spane);
		this.pack ();
	}
}