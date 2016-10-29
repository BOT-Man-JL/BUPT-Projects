import java.util.Scanner;

public class _14211288_李俊宁_1_Student
{
	private String studentNumber;
	private String studentName;
	private int markForMaths;
	private int markForEnglish;
	private int markForScience;

	public _14211288_李俊宁_1_Student(String number, String name)
	{
		studentName = name;
		studentNumber = number;
		markForMaths = -1;
		markForEnglish = -1;
		markForScience = -1;
	}

	public _14211288_李俊宁_1_Student()
	{
		markForMaths = -1;
		markForEnglish = -1;
		markForScience = -1;
	}

	public String getNumber()
	{
		return studentNumber;
	}

	public String getName()
	{
		return studentName;
	}

	public void enterMarks(int markForMaths,
						   int markForEnglish,
						   int markForScience)
	{
		this.markForMaths = markForMaths;
		this.markForEnglish = markForEnglish;
		this.markForScience = markForScience;
	}

	public int getMathsMark()
	{
		return markForMaths;
	}

	public int getEnglishMark()
	{
		return markForEnglish;
	}

	public int getScienceMark()
	{
		return markForScience;
	}

	public double calculateAverage()
	{
		return (markForEnglish +
				markForMaths +
				markForScience) / 3.0;
	}

	public String toString()
	{
		return "学号: " + studentNumber +
			   "\n姓名: " + studentName +
			   "\n数学成绩: " + markForMaths +
			   "\n英语成绩: " + markForEnglish +
			   "\n科技成绩: " + markForScience +
			   "\n平均成绩: " + calculateAverage();
	}
}

class StudentTest
{
	public static void main(String[] args)
	{
		Scanner input = new Scanner(System.in);
		System.out.print("请输入学生学号：");
		String studentNumber = input.next();
		System.out.print("请输入学生姓名：");
		String studentName = input.next();

		_14211288_李俊宁_1_Student student =
			new _14211288_李俊宁_1_Student(studentNumber, studentName);

		System.out.print("请输入学生三门课成绩（数学，英语，科学）：");
		input.nextLine();
		String[] strs = input.nextLine().split(",");
		try
		{
			student.enterMarks(Integer.parseInt(strs[0].trim()),
							   Integer.parseInt(strs[1].trim()),
							   Integer.parseInt(strs[2].trim()));

			System.out.println("学生信息如下: ");
			System.out.println(student);
		}
		catch (Exception e)
		{
			System.out.println("Bad Input");
		}
	}
}