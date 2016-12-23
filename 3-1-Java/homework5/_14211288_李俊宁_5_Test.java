
package homework5;

import java.util.Scanner;

public class _14211288_李俊宁_5_Test
{
	static public void main (String[] args)
	{
		_14211288_李俊宁_5_Factory factory = new _14211288_李俊宁_5_Factory ();
		factory.initEmployees ();

		System.out.println ("菜单如下，请输入 1~5 代表您要执行的操作：\n" +
							"1. 输入要查询的员工社会保险号，输出员工信息；\n" +
							"2. 对于已经存在于员工列表中的一个员工，输入更新信息来更新列表中的员工对象；\n" +
							"3. 对于已经存在于员工列表中的一个员工，将其从HashMap中删除；\n" +
							"4. 打印全部员工的信息；\n" +
							"5. 退出程序");

		Scanner scanner = new Scanner (System.in);
		Boolean isQuit = false;
		while (!isQuit)
		{
			System.out.print ("请输入您的操作: ");
			try
			{
				switch (Integer.parseInt (scanner.next ().trim ()))
				{
					case 1:
					{
						System.out.print ("输入要查询的员工社会保险号：");
						_14211288_李俊宁_5_Employee stu =
							factory.getEmployee (scanner.next ().trim ());

						if (stu != null)
						{
							System.out.println ("查询的员工信息：");
							System.out.println (stu.toString ());
						}
						else
							System.out.println ("无此员工");
						break;
					}
					case 2:
					{
						System.out.print ("输入要更新的员工社会保险号：");
						String ssn = scanner.next ().trim ();

						System.out.print ("输入员工名：");
						String firstName = scanner.next ().trim ();

						System.out.print ("输入员工姓：");
						String lastName = scanner.next ().trim ();
						
						System.out.print ("请输入员工类别：\n" +
										  "0 - SalaridEmployee\n" +
										  "1 - HourlyEmployee\n" +
										  "2 - CommisionEmployee\n" +
										  "3 - BasePlusCommisionEmployee\n");
						int type = Integer.parseInt (scanner.next ().trim ());

						_14211288_李俊宁_5_Employee stu;

						switch (type)
						{
						case 0:
						{
							System.out.print ("输入weeklySalary：");
							int weeklySalary = Integer.parseInt (scanner.next ().trim ());

							stu = factory.updateEmployee (ssn,
								new _14211288_李俊宁_5_SalaridEmployee(
									firstName, lastName, ssn,
									weeklySalary
								));
							break;
						}
						case 1:
						{
							System.out.print ("输入wage：");
							int wage = Integer.parseInt (scanner.next ().trim ());

							System.out.print ("输入hours：");
							int hours = Integer.parseInt (scanner.next ().trim ());

							stu = factory.updateEmployee (ssn,
								new _14211288_李俊宁_5_HourlyEmployee(
									firstName, lastName, ssn,
									wage, hours
								));
							break;
						}
						case 2:
						{
							System.out.print ("输入grossSales：");
							int grossSales = Integer.parseInt (scanner.next ().trim ());

							System.out.print ("输入commissionRate：");
							double commissionRate = Double.parseDouble (scanner.next ().trim ());

							stu = factory.updateEmployee (ssn,
								new _14211288_李俊宁_5_CommisionEmployee(
									firstName, lastName, ssn,
									grossSales, commissionRate
								));
							break;
						}
						case 3:
						{
							System.out.print ("输入grossSales：");
							int grossSales = Integer.parseInt (scanner.next ().trim ());

							System.out.print ("输入commissionRate：");
							double commissionRate = Double.parseDouble (scanner.next ().trim ());

							System.out.print ("输入baseSalary：");
							int baseSalary = Integer.parseInt (scanner.next ().trim ());

							stu = factory.updateEmployee (ssn,
								new _14211288_李俊宁_5_BasePlusCommisionEmployee(
									firstName, lastName, ssn,
									grossSales, commissionRate, baseSalary
								));
							break;
						}
						default:
							throw new Exception ();
						}

						if (stu != null)
						{
							System.out.println ("更新的员工信息：");
							System.out.println (stu.toString ());
						}
						else
							System.out.println ("无此员工");
						break;
					}
					case 3:
					{
						System.out.print ("输入要删除的员工社会保险号：");
						_14211288_李俊宁_5_Employee stu =
							factory.deleteEmployee(scanner.next().trim());

						if (stu != null)
						{
							System.out.println ("删除的员工信息：");
							System.out.println (stu.toString ());
						}
						else
							System.out.println ("无此员工");
						break;
					}
					case 4:
					{
						System.out.println ("打印全部员工的信息：");
						factory.printEmployees ();
						break;
					}
					case 5:
					{
						isQuit = true;
						System.out.println ("退出程序");
						break;
					}
					default:
					{
						throw new Exception ();
					}
				}
			}
			catch (Exception e)
			{
				System.out.println ("非法输入");
			}
		}
	}
}
