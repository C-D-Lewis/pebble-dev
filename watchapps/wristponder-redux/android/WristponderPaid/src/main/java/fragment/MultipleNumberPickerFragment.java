package fragment;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.widget.TextView;

import java.util.ArrayList;

public class MultipleNumberPickerFragment extends DialogFragment {

    private CharSequence[] numbers;
    private TextView numberView;
    private TextView nameView;
    private String name;

    public MultipleNumberPickerFragment(ArrayList<String> list, TextView nameView, String name, TextView numberView) {
        this.numberView = numberView;
        this.name = name;
        this.nameView = nameView;

        numbers = new CharSequence[list.size()];
        for(int i = 0; i < list.size(); i++) {
            numbers[i] = list.get(i);
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setTitle("Choose a number");
        builder.setItems(numbers, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // Deferred UI update if a choice is made, not if not
                nameView.setText(name);
                numberView.setText(numbers[which]);
            }

        });
        return builder.create();
    }
}
