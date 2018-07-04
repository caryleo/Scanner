package Question2;

import java.util.*;

public class Array {
    String[] week = new String[]{"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    Integer[] numbers = new Integer[20];
    // array sums will be initialized when needed
    Integer[] sums;
    public void setArray(){
        // use random to specify every element of array numbers
        Random random = new Random(20);
        for(int i = 0; i < this.numbers.length; i++) {
            this.numbers[i] = random.nextInt(45) - 20;
        }
    }
    public Integer minValue(Integer[] n){
        Integer min = n[0];
        for(Integer x : n){
            // use an iterator to traverse the array to find the value
            if(x < min){
                min = x;
            }
        }
        return min;
    }
    public void cumulativeSums(){
        Integer sum = 0;
        sums = new Integer[this.numbers.length];
        for(int i = 0; i < sums.length; i++){
            sum += numbers[i];
            sums[i] = sum;
        }
    }
    public static void main(String[] args){
        Array test = new Array();
        System.out.println("the string array " + Arrays.toString(test.week));
        test.setArray();
        System.out.println("the int array " + Arrays.toString(test.numbers));
        System.out.println("the minimum value " + test.minValue(test.numbers));
        test.cumulativeSums();
        System.out.println("the sum array " + Arrays.toString(test.sums));
    }
}
