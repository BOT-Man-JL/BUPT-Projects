import java.util.Scanner;

public class _14211288_李俊宁_1_StudentList
{
    private _14211288_李俊宁_1_Student[] list;
    private int total;

    public _14211288_李俊宁_1_StudentList(int length)
	{
        total = 0;
        list = new _14211288_李俊宁_1_Student[length];
    }

    public boolean add(_14211288_李俊宁_1_Student stu)
	{
        if (total == list.length)
            return false;
        else
		{
            list[total++] = stu;
            return true;
        }
    }

    public boolean remove(int no)
	{
		if (no > total || no <= 0)
			return false;

		int j = 0;
		for(int i = 0; i < total; i++)
			if (i != no - 1)
			{
				if (j != i)
					list[j] = list[i];
				j++;
			}
		total--;
		list[total] = null;
		return true;
    }

    public boolean remove(String number)
	{
		int index = -1;
		for (int i = 0; index == -1 && i < total; i++)
			if (list[i].getNumber().equals(number))
				index = i;

		if (index == -1)
			return false;
		else
			return this.remove (index + 1);
    }

    public boolean isEmpty()
	{
        return total == 0;
    }

    public _14211288_李俊宁_1_Student getItem(int no)
	{
        if (no > total || no <= 0)
            return null;
        else
            return list[no - 1];
    }

    public _14211288_李俊宁_1_Student getItem(String number)
	{
        for(int i = 0; i < total; i++)
            if (list[i].getNumber().equals(number))
                return list[i];
        return null;
    }

    public int getTotal()
	{
        return total;
    }
}

class StudentListTest
{
    public static void main(String[] args)
	{
        System.out.println("菜单如下，请输入 1~8代表您要执行的操作：\n" +
						   "1. 增加1个学生  2. 根据学号删除学生  3. 根据位置删除学生\n" +
						   "4. 判断是否为空   5.根据位置返回学生   6.根据学号返回学生\n" +
						   "7. 输出全部学生信息  8.退出程序");

        _14211288_李俊宁_1_StudentList studentList =
			new _14211288_李俊宁_1_StudentList(10);

        Scanner scanner = new Scanner(System.in);
		boolean isQuit = false;
        while (!isQuit)
		{
			boolean isPrintAll = false;
            System.out.print("请输入您的操作: ");
			try
			{
				switch (Integer.parseInt(scanner.next().trim()))
				{
					case 1:
					{
						System.out.println("请输入学生信息：");
						System.out.print("学号：");
						String studentNumber = scanner.next();
						System.out.print("姓名：");
						String studentName = scanner.next();
						System.out.print("数学成绩: ");
						int markForMaths = Integer.parseInt(scanner.next().trim());
						System.out.print("英语成绩: ");
						int markForEnglish = Integer.parseInt(scanner.next().trim());
						System.out.print("科学成绩: ");
						int markForScience = Integer.parseInt(scanner.next().trim());

						_14211288_李俊宁_1_Student student =
							new _14211288_李俊宁_1_Student(studentNumber, studentName);
						student.enterMarks(markForMaths, markForEnglish, markForScience);
						studentList.add(student);

						isPrintAll = true;
						break;
					}
					case 2:
					{
						System.out.print("请输入学生学号：");

						if (studentList.remove(scanner.next()))
						{
							isPrintAll = true;
							System.out.println("删除成功");
						}
						else
							System.out.println("对不起，没有对应的学生");
						break;
					}
					case 3:
					{
						System.out.print("请输入学生学号：");

						if (studentList.remove(Integer.parseInt(scanner.next().trim())))
						{
							isPrintAll = true;
							System.out.println("删除成功");
						}
						else
							System.out.println("对不起，没有对应的学生");
						break;
					}
					case 4:
					{
						if (studentList.isEmpty())
							System.out.println("为空");
						else
							System.out.println("非空");
						break;
					}
					case 5:
					{
						System.out.print("请输入学生位置：");
						_14211288_李俊宁_1_Student stu =
							studentList.getItem(Integer.parseInt(scanner.next().trim()));

						if (stu != null)
						{
							System.out.println("学生信息如下：");
							System.out.println(stu);
						}
						else
							System.out.println("对不起，没有相应的学生");
						break;
					}
					case 6:
					{
						System.out.print("请输入学生学号：");
						_14211288_李俊宁_1_Student stu =
							studentList.getItem(scanner.next());

						if (stu != null)
						{
							System.out.println("学生信息如下：");
							System.out.println(stu);
						}
						else
							System.out.println("对不起，没有相应的学生");
						break;
					}
					case 7:
					{
						isPrintAll = true;
						break;
					}
					case 8:
					{
						isQuit = true;
						break;
					}
					default:
					{
						throw new Exception();
					}
				}

				// Print All Here
				if (isPrintAll)
				{
					System.out.println("---目前有" +studentList.getTotal() + "个学生，信息为---");
					for (int i = 0; i < studentList.getTotal(); i++)
						System.out.println(studentList.getItem(i + 1));
				}
			}
			catch (Exception e)
			{
				System.out.println("Bad Input");
			}
        }
    }
}