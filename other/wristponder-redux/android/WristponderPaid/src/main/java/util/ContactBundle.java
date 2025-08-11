package util;

public class ContactBundle implements Comparable<ContactBundle> {
	
	public String
		name,
		number;
	
	public ContactBundle(String name, String number) {
		this.name = name;
		this.number = number;
	}
	
	@Override
	public int compareTo(ContactBundle b) {
	    return this.name.compareTo(b.name);	//Sort based on name field
	}

}
