
package homework7;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.Map.Entry;
import java.awt.*;
import javax.swing.*;

public class _14211288_李俊宁_7_StrategiesUI extends JFrame
{
	private _14211288_李俊宁_7_Controller controller;
	private JPanel panel;

	public _14211288_李俊宁_7_StrategiesUI (_14211288_李俊宁_7_Controller _controller)
	{
		controller = _controller;
		initUI ();
	}
	
	private void initUI ()
	{
		update ();

		// Misc
		setTitle ("定价策略维护模块");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (DISPOSE_ON_CLOSE);
	}

	private void update ()
	{
		HashMap<Integer, _14211288_李俊宁_7_IPricingStrategy> strategies =
			controller.getStrategies ();
		int[] categoryInTable = new int[strategies.size ()];
		String[] categoryStrings = controller.getCategoryStrings ();
		String[] strategyTypeStrings = controller.getStrategyTypeStrings ();
		
		// Clear previous canvas
		if (panel != null)
			this.remove (panel);

		panel = new JPanel ();
		panel.setLayout (new BoxLayout (panel, BoxLayout.Y_AXIS));

		// Table
		String[] columnNames = {"策略编号",
								"策略名称",
								"策略类型",
								"适用图书类型",
								"折扣百分比/每本优惠额度/组合信息"};
		String[][] tableData = new String[strategies.size ()][5];

		int index = 0;
		for (Entry<Integer, _14211288_李俊宁_7_IPricingStrategy> strategyPair :
				strategies.entrySet ())
		{
			Integer category = strategyPair.getKey ();
			_14211288_李俊宁_7_IPricingStrategy strategy = strategyPair.getValue ();

			tableData[index] = new String[]
			{
				strategy.getIndex (),
				strategy.getName (),
				strategyTypeStrings[strategy.getType () - 1],
				categoryStrings[category - 1],
				strategy.getDescription ()
			};
			categoryInTable[index] = category;
			index++;
		}

		JTable table = new JTable (tableData, columnNames);
		table.setSelectionMode (ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		panel.add (new JScrollPane (table));
		
		// Footer
		JPanel fpanel = new JPanel ();

		// Index
		fpanel.add (new JLabel ("策略编号"));
		JTextField tIndex = new JTextField (10);
		fpanel.add (tIndex);

		// Name
		fpanel.add (new JLabel ("策略名称"));
		JTextField tName = new JTextField (10);
		fpanel.add (tName);

		// Category
		fpanel.add (new JLabel ("适用图书类型"));
		JComboBox<String> cCategory =
			new JComboBox<String> (categoryStrings);
		fpanel.add (cCategory);

		// Composite Panel
		JPanel vPanel = new JPanel ();
		vPanel.setLayout (new BoxLayout (vPanel, BoxLayout.Y_AXIS));
		
		// For Simple
		JPanel vPanel1 = new JPanel ();

		// Count
		vPanel1.add (new JLabel ("折扣百分比/每本优惠额度"));
		JTextField tCount = new JTextField (10);
		vPanel1.add (tCount);

		// Button Add FlatRate
		JButton bAddFlatRate = new JButton("+ 绝对值策略");
		bAddFlatRate.addActionListener ((e) ->
		{
			try
			{
				_14211288_李俊宁_7_IPricingStrategy strategy =
					new _14211288_李俊宁_7_FlatRateStrategy (
						tIndex.getText (),
						tName.getText (),
						Double.parseDouble (tCount.getText ())
					);
				controller.setStrategy (cCategory.getSelectedIndex () + 1,
										strategy);
				update ();
			}
			catch (NumberFormatException ex)
			{
				System.out.println ("绝对值 非法输入");
			}
			catch (Exception ex)
			{
				System.out.println (ex.getMessage ());
			}
		});
		vPanel1.add (bAddFlatRate);

		// Button Add Percentage
		JButton bAddPercentage = new JButton("+ 百分比策略");
		bAddPercentage.addActionListener ((e) ->
		{
			try
			{
				_14211288_李俊宁_7_IPricingStrategy strategy =
					new _14211288_李俊宁_7_PercentageStrategy (
						tIndex.getText (),
						tName.getText (),
						Double.parseDouble (tCount.getText ())
					);
				controller.setStrategy (cCategory.getSelectedIndex () + 1,
										strategy);
				update ();
			}
			catch (NumberFormatException ex)
			{
				System.out.println ("百分比 非法输入");
			}
			catch (Exception ex)
			{
				System.out.println (ex.getMessage ());
			}
		});
		vPanel1.add (bAddPercentage);

		vPanel.add (vPanel1);

		// For Composite & Delete & Quit
		JPanel vPanel2 = new JPanel ();

		// Hint
		vPanel2.add (new JLabel ("使用 Ctrl/Shift 多选"));
		
		// Button Add Composite
		JButton bAddComposite = new JButton("组合选中策略为新策略");
		bAddComposite.addActionListener ((e) ->
		{
			_14211288_李俊宁_7_CompositeStrategy strategy =
				new _14211288_李俊宁_7_CompositeStrategy (
					tIndex.getText (),
					tName.getText ()
				);

			int[] rowIndices = table.getSelectedRows();
			for (int i : rowIndices)
				strategy.add (strategies.get (categoryInTable[i]));

			if (rowIndices.length > 0)
			{
				controller.setStrategy (cCategory.getSelectedIndex () + 1,
										strategy);
				update ();
			}
		});
		vPanel2.add (bAddComposite);

		// Button Delete
		JButton bDelete = new JButton("删除选中策略");
		bDelete.addActionListener ((e) ->
		{
			int[] rowIndices = table.getSelectedRows();
			for (int i : rowIndices)
				controller.deleteStrategy (categoryInTable[i]);

			if (rowIndices.length > 0)
				update ();
		});
		vPanel2.add (bDelete);

		// Button Cancel
		JButton bCancel = new JButton("离开");
		bCancel.addActionListener ((e) ->
		{
			this.dispose();
		});
		vPanel2.add (bCancel);

		vPanel.add (vPanel2);

		fpanel.add (vPanel);
		panel.add (fpanel);
		
		// Add Panel
		this.add (panel);
		this.pack ();
	}
}