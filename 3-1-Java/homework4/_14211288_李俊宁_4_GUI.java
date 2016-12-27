
package homework4;

import java.util.ArrayList;
import java.awt.*;
import javax.swing.*;

public class _14211288_李俊宁_4_GUI extends JFrame
{
	private ArrayList<_14211288_李俊宁_4_Employee> employees;

	class CommisionEmployeeDialog extends JDialog
	{
		public CommisionEmployeeDialog (Frame parent)
		{
			super (parent);
			initUI ();
		}

		private void initUI ()
		{
			JPanel panel = new JPanel ();
			panel.setLayout (new GridLayout (6, 2));

			// First Name
			panel.add (new JLabel ("First Name"));
			JTextField tFirstName = new JTextField (10);
			panel.add (tFirstName);

			// Last Name
			panel.add (new JLabel ("Last Name"));
			JTextField tLastName = new JTextField (10);
			panel.add (tLastName);

			// Social Security Number
			panel.add (new JLabel ("Social Security Number"));
			JTextField tSSN = new JTextField (10);
			panel.add (tSSN);

			// Gross Sales
			panel.add (new JLabel ("Gross Sales"));
			JTextField tGrossSales = new JTextField (10);
			panel.add (tGrossSales);

			// Commission Rate
			panel.add (new JLabel ("Commission Rate"));
			JTextField tCommissionRate = new JTextField (10);
			panel.add (tCommissionRate);

			// Buttons
			JButton bAdd = new JButton("Add");
			bAdd.addActionListener ((e) ->
			{
				try
				{
					employees.add (new _14211288_李俊宁_4_CommisionEmployee (
						tFirstName.getText(),
						tLastName.getText(),
						tSSN.getText(),
						Integer.parseInt(tGrossSales.getText()),
						Double.parseDouble(tCommissionRate.getText())
					));
					this.dispose();
				}
				catch (Exception ex)
				{
					System.out.println ("非法输入");
				}
			});
			JButton bCancel = new JButton("Cancel");
			bCancel.addActionListener ((e) ->
			{
				this.dispose();
			});
			panel.add (bAdd);
			panel.add (bCancel);

			// Add Panel
			this.add (panel);
			this.pack ();

			// Misc
			setTitle ("Create Commision Employee");
			setDefaultCloseOperation (DISPOSE_ON_CLOSE);
			setLocationRelativeTo (getParent ());
		}
	}

	class BasePlusCommisionEmployeeDialog extends JDialog
	{
		public BasePlusCommisionEmployeeDialog (Frame parent)
		{
			super (parent);
			initUI ();
		}

		private void initUI ()
		{
			JPanel panel = new JPanel ();
			panel.setLayout (new GridLayout (7, 2));

			// First Name
			panel.add (new JLabel ("First Name"));
			JTextField tFirstName = new JTextField (10);
			panel.add (tFirstName);

			// Last Name
			panel.add (new JLabel ("Last Name"));
			JTextField tLastName = new JTextField (10);
			panel.add (tLastName);

			// Social Security Number
			panel.add (new JLabel ("Social Security Number"));
			JTextField tSSN = new JTextField (10);
			panel.add (tSSN);

			// Gross Sales
			panel.add (new JLabel ("Gross Sales"));
			JTextField tGrossSales = new JTextField (10);
			panel.add (tGrossSales);

			// Commission Rate
			panel.add (new JLabel ("Commission Rate"));
			JTextField tCommissionRate = new JTextField (10);
			panel.add (tCommissionRate);

			// Commission Rate
			panel.add (new JLabel ("Base Salary"));
			JTextField tBaseSalary = new JTextField (10);
			panel.add (tBaseSalary);

			// Buttons
			JButton bAdd = new JButton("Add");
			bAdd.addActionListener ((e) ->
			{
				try
				{
					employees.add (new _14211288_李俊宁_4_BasePlusCommisionEmployee (
						tFirstName.getText(),
						tLastName.getText(),
						tSSN.getText(),
						Integer.parseInt(tGrossSales.getText()),
						Double.parseDouble(tCommissionRate.getText()),
						Integer.parseInt(tBaseSalary.getText())
					));
					this.dispose();
				}
				catch (Exception ex)
				{
					System.out.println ("非法输入");
				}
			});
			JButton bCancel = new JButton("Cancel");
			bCancel.addActionListener ((e) ->
			{
				this.dispose();
			});
			panel.add (bAdd);
			panel.add (bCancel);

			// Add Panel
			this.add (panel);
			this.pack ();

			// Misc
			setTitle ("Create Base Plus Commision Employee");
			setDefaultCloseOperation (DISPOSE_ON_CLOSE);
			setLocationRelativeTo (getParent ());
		}	
	}

	class AverageEarningSearchDialog extends JDialog
	{
		public AverageEarningSearchDialog (Frame parent)
		{
			super (parent);
			initUI ();
		}

		private void initUI ()
		{
			double avg = 0.0;
			if (employees.size () > 0)
			{
				for (_14211288_李俊宁_4_Employee employee : employees)
					avg += employee.earning ();
				avg /= employees.size ();
			}

			JPanel panel = new JPanel();
			panel.setLayout (new BoxLayout (panel, BoxLayout.Y_AXIS));

			// Content
			panel.add (new JLabel("Average Earning"));
			panel.add (new JLabel("" + avg));

			// Add Panel
			this.add (panel);
			this.pack ();
			
			// Misc
			setTitle ("Average Earning");
			setDefaultCloseOperation (DISPOSE_ON_CLOSE);
			setLocationRelativeTo (getParent ());
		}	
	}

	public _14211288_李俊宁_4_GUI ()
	{
		initUI ();
		employees = new ArrayList<_14211288_李俊宁_4_Employee> ();
	}

	private void initUI ()
	{
		// Menu
		initMenu ();

		// Content
		this.add (new JLabel ("Swing GUI - 员工信息的录入和查询"));
		setSize (640, 200);

		// Misc
		setTitle ("员工信息的录入和查询");
		setLocationRelativeTo (null);
		setDefaultCloseOperation (EXIT_ON_CLOSE);
	}

	private void initMenu ()
	{
		// Sub Menu 1-1
		JMenuItem menuCommisionEmployee =
			new JMenuItem ("CommisionEmployee");
		menuCommisionEmployee.addActionListener ((e) ->
		{
			CommisionEmployeeDialog dialog =
				new CommisionEmployeeDialog (this);
			dialog.setVisible(true);
		});

		// Sub Menu 1-2
		JMenuItem menuBasePlusCommisionEmployee =
			new JMenuItem ("BasePlusCommisionEmployee");
		menuBasePlusCommisionEmployee.addActionListener ((e) ->
		{
			BasePlusCommisionEmployeeDialog dialog =
				new BasePlusCommisionEmployeeDialog (this);
			dialog.setVisible(true);
		});

		// Main Menu 1
		JMenu menuEmployeeInfoInput =
			new JMenu ("EmployeeInfoInput");
		menuEmployeeInfoInput.add (menuCommisionEmployee);
		menuEmployeeInfoInput.add (menuBasePlusCommisionEmployee);

		// Sub Menu 2-1
		JMenuItem menuAverageEarningSearch =
			new JMenuItem ("AverageEarningSearch");
		menuAverageEarningSearch.addActionListener ((e) ->
		{
			AverageEarningSearchDialog dialog =
				new AverageEarningSearchDialog (this);
			dialog.setVisible(true);
		});

		// Main Menu 2
		JMenu menuSearch = new JMenu ("Search");
		menuSearch.add (menuAverageEarningSearch);

		// Menu Bar
		JMenuBar menu = new JMenuBar ();
		menu.add (menuEmployeeInfoInput);
		menu.add (menuSearch);
		this.setJMenuBar (menu);
	}

	public static void main (String[] args)
	{
		// Start
		EventQueue.invokeLater (() ->
		{
			_14211288_李俊宁_4_GUI gui = new _14211288_李俊宁_4_GUI ();
			gui.setVisible (true);
		});
	}
}
