
package homework7;

import java.util.ArrayList;
import java.awt.*;
import javax.swing.*;

public class _14211288_李俊宁_7_MainUI extends JFrame
{
	private _14211288_李俊宁_7_Controller controller;

	public _14211288_李俊宁_7_MainUI (_14211288_李俊宁_7_Controller _controller)
	{
		controller = _controller;
		initUI ();
	}

	private void initUI ()
	{
		JPanel panel = new JPanel ();

		// AddBook
		JButton bAddBook = new JButton("图书目录的添加模块");
		bAddBook.addActionListener ((e) ->
		{
			_14211288_李俊宁_7_AddBookUI addBookUI =
				new _14211288_李俊宁_7_AddBookUI (controller);
			addBookUI.setVisible (true);
		});
		panel.add (bAddBook);

		// Strategies
		JButton bStrategies = new JButton("定价策略维护模块");
		bStrategies.addActionListener ((e) ->
		{
			_14211288_李俊宁_7_StrategiesUI strategiesUI =
				new _14211288_李俊宁_7_StrategiesUI (controller);
			strategiesUI.setVisible (true);
		});
		panel.add (bStrategies);
		
		// Buy
		JButton bBuy = new JButton("购书模块");
		bBuy.addActionListener ((e) ->
		{
			_14211288_李俊宁_7_BuyUI buyUI =
				new _14211288_李俊宁_7_BuyUI (controller);
			buyUI.setVisible (true);
		});
		panel.add (bBuy);
		
		// Add Panel
		this.add (panel);
		this.pack ();

		// Misc
		setTitle ("电子商务网站的购物车系统");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (EXIT_ON_CLOSE);
	}
	
	private static void seedBook (
		_14211288_李俊宁_7_Controller controller) throws Exception
	{
		ArrayList<_14211288_李俊宁_7_BookSpecification> books =
			new ArrayList<_14211288_李俊宁_7_BookSpecification> ();
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"978-7-302-2", "UML 与模式应用", 18, 2));
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"978-7-312-3", "Java 与模式", 34, 1));
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"968-6-302-1", "HeadFirst 设计模式", 58, 1));
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"958-1-302-2", "爱丽丝历险记", 30, 3));
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"900-7-392-2", "煲汤大全", 20, 4));
		books.add (new _14211288_李俊宁_7_BookSpecification (
				"N/A", "其他类型书籍", 100, 5));

		for (_14211288_李俊宁_7_BookSpecification book : books)
			controller.addBook (book);
	}

	private static void seedStrategy (
		_14211288_李俊宁_7_Controller controller) throws Exception
	{
		_14211288_李俊宁_7_IPricingStrategy d1 =
			new _14211288_李俊宁_7_FlatRateStrategy (
				"Discount001", "绝对值优惠策略 1", 1);
		_14211288_李俊宁_7_IPricingStrategy d2 =
			new _14211288_李俊宁_7_PercentageStrategy (
				"Discount002", "百分比折扣策略 1", 7);
		_14211288_李俊宁_7_IPricingStrategy d3 =
			new _14211288_李俊宁_7_PercentageStrategy (
				"Discount003", "百分比折扣策略 2", 3);
		_14211288_李俊宁_7_CompositeStrategy d4 =
			new _14211288_李俊宁_7_CompositeStrategy (
				"Discount004", "顾客最优策略 1");
		d4.add (d1);
		d4.add (d3);

		controller.setStrategy (1, d3);
		controller.setStrategy (2, d1);
		controller.setStrategy (3, d2);
		controller.setStrategy (4, d4);
	}

	private static void seedSale (
		_14211288_李俊宁_7_Controller controller) throws Exception
	{
		controller.buyBook ("978-7-302-2", 2);
		controller.buyBook ("978-7-312-3", 2);
		controller.buyBook ("968-6-302-1", 1);
		controller.buyBook ("958-1-302-2", 3);
		controller.buyBook ("900-7-392-2", 3);
	}

	public static void main (String[] args)
	{
		// Shared Object
		_14211288_李俊宁_7_Controller controller =
			new _14211288_李俊宁_7_Controller ();

		// Seed Data if args.length > 0
		try
		{
			if (args.length > 0)
			{
				seedBook (controller);
				seedStrategy (controller);
				seedSale (controller);
			}
		}
		catch (Exception ex)
		{
			System.out.println (ex.getMessage ());
		}

		System.out.println ("Hint: Use any Argument to Start to Init data :-)");

		// Start
		EventQueue.invokeLater (() ->
		{
			_14211288_李俊宁_7_MainUI gui =
				new _14211288_李俊宁_7_MainUI (controller);
			gui.setVisible (true);
		});
	}
}